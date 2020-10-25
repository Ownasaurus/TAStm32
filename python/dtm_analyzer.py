import sys

def main():
    pollNumber = 0
    try:
        file = sys.argv[1]
    except:
        print(f'Usage {sys.argv[0]} <movie file>')
        sys.exit()
    with open(file, 'rb') as f:
        header = f.read(0x100) #skip the header
        with open('output.dtm', 'wb') as o:
            o.write(header)
            o.write(f.read(32)) # assume first poll is M. see if this fixes it
            pollNumber += 1
            while(True):
                S = f.read(8) # read the S poll of controller 1
                f.read(24) # dump the S polls of controllers 2-4
                M = f.read(8) # read the S poll of controller 1
                extra = f.read(24) # dump the S polls of controllers 2-4
                pollNumber += 2
                if S and M:
                    o.write(M)
                    o.write(extra)
                    o.write(M)
                    o.write(extra)
                    if S != M:
                        print(f'Mismatch found between polls #{pollNumber-1} and #{pollNumber}')
                        offset = 0x100 + (pollNumber-2)*32
                        print('S = '+' '.join('{:02X}'.format(a) for a in S))
                        print('M = '+' '.join('{:02X}'.format(a) for a in M))
                        print('File offset = 0x'+''.join('{:08X}'.format(offset)))
                else:
                    break
    
if __name__ == '__main__':
    main()