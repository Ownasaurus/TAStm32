#!/usr/bin/env python3

class RunData():
    def __init__(self, data, id):
        self._rawbuffer = data
        self._blankframes = 0
        self._extraframes = {}
        self._blankframe = b'\x00'*len(data[0])
        self.frame = 0
        self.framemax = len(self._rawbuffer)
        self.id = id
        self.buffer = []
        self.update()
        self.transitions = []

    def reset(self):
        self.frame = 0

    def update(self):
        self.buffer = []
        for _ in range(self._blankframes):
            self.buffer.append(self._blankframe)
        self.buffer.extend(self._rawbuffer)
        for frame in self._extraframes:
            self.buffer.insert(self._blankframes+frame, self._blankframe)
        self.framemax = len(self.buffer)

    def setBlankFrames(self, count):
        count = int(count)
        if count < 0:
            print("Could not set blank frames to negative value")
        else:
            self._blankframes = count

    def addExtraFrames(self, index, count):
        index = int(index)
        count = int(count)
        if index < 0:
            print("Could not add frames before movie start")
        elif index > len(self._rawbuffer):
            print("Could not add frames after movie end")
        else:
            if count <= 0:
                print("Count must be a positve number greater than 0")
            else:
                self._extraframes[index] = self._extraframes.get(index, 0) + count

    def removeExtraFrames(self, index, count):
        index = int(index)
        if index < 0:
            print("Could not remove frames before movie start")
        elif index > len(self._rawbuffer):
            print("Could not remove frames after movie end")
        else:
            if count == 'all':
                self._extraframes[index] = 0
            else:
                count = int(count)
                current = self._extraframes.get(index, 0)
                if count <= 0 or count > current:
                    print("Count must be a positve number greater than 0")
                    print("and not more than current amount")
                else:
                    self._extraframes[index] = self._extraframes.get(index, 0) - count
