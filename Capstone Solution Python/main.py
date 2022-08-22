# Import the irig106 package
import Py106

# Create IRIG IO object
IrigIO  = Py106.packet.IO()

# Open data file for reading
IrigIO.open("test.ch10", 1)

# Read IRIG headers
for pkt_hdr in IrigIO.packet_headers():
    IrigIO.read_data()
    print ("Ch ID %3i  %s" % (IrigIO.read_next_header(), Py106.packet.DataType.type_name(pkt_hdr.__str__)))
        
IrigIO.close()