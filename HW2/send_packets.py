from subprocess import *




def create_packets(packet_num):
    packets = ["packet"+str(i%10)+" " for i in range(packet_num)]
    
    packets = "".join(packets)
    packets+="\n"
    print(packets)
    return packets
while(True):
    inp=input()
    packets = create_packets(int(inp))
    print(packets)

