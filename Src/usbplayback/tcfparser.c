#include "yaml/yaml.h"
#include "TASRun.h"
#include "fatfs.h"

enum state_value {
	ACCEPT_KEY, ACCEPT_VALUE, COLLECTION_KEY, COLLECTION_VALUE, STOP, ERR
};

struct parser_state {
	TASRun *run;
	enum state_value state;
	int accepted;
	int error;
	char key[20];
	char value[128];
	char collection[20];
	int collection_index;
};

// state machine to do parsing

int do_parse_event(struct parser_state *s, yaml_event_t *event) {
	s->accepted = 0;
	if (event->type == YAML_STREAM_END_EVENT) {
		s->state = STOP;
		return 1;
	}
	switch (s->state) {

	case ACCEPT_KEY:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			strcpy(s->key, (char *) event->data.scalar.value);
			s->state = ACCEPT_VALUE;
			break;
		default:
			fprintf(stderr, "Unexpected event while getting key :%d\n", event->type);
			//s->state = ERROR;
			break;
		}
		break;

	case ACCEPT_VALUE:
		switch (event->type) {

		case YAML_SEQUENCE_START_EVENT:
			printf("Begin collection\n");
			strcpy(s->collection, s->key);
			s->state = COLLECTION_KEY;
			s->collection_index = 0;
			break;

		case YAML_SCALAR_EVENT:
			strcpy(s->value, (char *) event->data.scalar.value);
			if (strcmp(s->key, "inputFile") == 0)
				strcpy(s->run->inputFile, s->value);
			else if (strcmp(s->key, "numControllers") == 0)
				TASRunSetNumControllers(atoi(s->value));
			else if (strcmp(s->key, "numDataLanes") == 0)
				TASRunSetNumDataLanes(atoi(s->value));
			else if (strcmp(s->key, "dpcmFix") == 0)
				s->run->dpcmFix = atoi(s->value);
			else if (strcmp(s->key, "clockFix") == 0)
				s->run->clockFix = atoi(s->value);
			else if (strcmp(s->key, "overread") == 0)
				s->run->overread = atoi(s->value);
			else if (strcmp(s->key, "blank") == 0)
				s->run->blank = atoi(s->value);
			else if (strcmp(s->key, "multitap") == 0)
				s->run->multitap = atoi(s->value);
			else if (strcmp(s->key, "console") == 0) {
				if (strcmp(s->value, "snes") == 0) {
					TASRunSetConsole(CONSOLE_SNES);
					SetSNESMode();
					TASRunSetNumControllers( 2);
					TASRunSetNumDataLanes(4);
				} else if (strcmp(s->value, "nes") == 0) {
					TASRunSetConsole(CONSOLE_NES);
					SetNESMode();
					TASRunSetNumControllers(2);
					TASRunSetNumDataLanes(1);
				} else if (strcmp(s->value, "n64") == 0) {
					TASRunSetConsole(CONSOLE_N64);
					SetN64Mode();
				} else if (strcmp(s->value, "gc") == 0) {
					TASRunSetConsole(CONSOLE_GC);
					SetN64Mode();
				} else {
					//??
				}
			} else {
				fprintf(stderr, "Ignoring unknown key: %s\n", s->key);
			}
			s->state = ACCEPT_KEY;
			break;
		default:
			fprintf(stderr, "Unexpected event while getting key %s's value: %d\n", s->key, event->type);
			return 0;
			break;
		}
		break;

	case COLLECTION_KEY:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			strcpy(s->key, (char *) event->data.scalar.value);
			s->state = COLLECTION_VALUE;
			break;

		case YAML_MAPPING_END_EVENT:
			s->collection_index++;
			s->state = COLLECTION_KEY;
			break;

		case YAML_SEQUENCE_END_EVENT:
			//s->collection_index++;
			s->state = ACCEPT_KEY;
			printf("End collection\n");
			break;
		default:
			fprintf(stderr, "Unexpected event while getting key :%d\n", event->type);
			//s->state = ERROR;
			break;
		}
		break;

	case COLLECTION_VALUE:
		switch (event->type) {

		case YAML_SCALAR_EVENT:
			strcpy(s->value, (char *) event->data.scalar.value);
			if (strcmp(s->collection, "transitions") == 0) {
				if (strcmp(s->key, "type") == 0) {
					//printf("%s %d type is %s\n", s->collection, s->collection_index, s->value);
					if (strcmp(s->value, "normal") == 0)
						s->run->transitions_dpcm[s->collection_index].type = TRANSITION_NORMAL;
					if (strcmp(s->value, "ace") == 0)
						s->run->transitions_dpcm[s->collection_index].type = TRANSITION_ACE;
					if (strcmp(s->value, "resetsoft") == 0)
						s->run->transitions_dpcm[s->collection_index].type = TRANSITION_RESET_SOFT;
					if (strcmp(s->value, "resethard") == 0)
						s->run->transitions_dpcm[s->collection_index].type = TRANSITION_RESET_HARD;
				} else if (strcmp(s->key, "frameno") == 0)
					s->run->transitions_dpcm[s->collection_index].frameno = atoi(s->value); //printf("%s %d frameno %d\n", s->collection, s->collection_index, atoi(s->value));
				else {
					fprintf(stderr, "Ignoring unknown key: %s\n", s->key);
				}
			}
			s->state = COLLECTION_KEY;

			break;
		default:
			//fprintf(stderr, "Unexpected event while getting key %s's value: %d\n", s->key, event->type);
			return 0;
			break;
		}
		break;
	default:
		break;
	}
	return 1;
}

int read_handler(void *ext, unsigned char *buffer, size_t size, size_t *length) {
	/* ... */
	FIL *tcf = (FIL *) ext;
	static FRESULT res;

	res = f_read(tcf, buffer, size, length);

	return res == FR_OK ? 1 : 0;
}

int load_tcf(char *filename) {
	FIL tcf;
	static FRESULT res;
	yaml_parser_t parser;
	yaml_event_t event; /* New variable */
	struct parser_state state = { .state = ACCEPT_KEY, .accepted = 0, .error = 0, .collection_index = 0, .run = tasrun };

	res = f_open(&tcf, filename, FA_READ);
	if (res == FR_OK) {
		/* Initialize parser */
		if (!yaml_parser_initialize(&parser))
			return 1;

		/* Set input file */
		yaml_parser_set_input(&parser, read_handler, (void *) &tcf);

		do {
			if (!yaml_parser_parse(&parser, &event)) {
				goto error;
			}
			if (!do_parse_event(&state, &event)) {
				goto error;
			}

			yaml_event_delete(&event);
		} while (state.state != STOP);

		yaml_parser_delete(&parser);
		f_close(&tcf);
		return 1;

	} else
		return 0;

	error: yaml_parser_delete(&parser);
	f_close(&tcf);
	return 0;

}

