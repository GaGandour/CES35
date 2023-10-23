match2 = parser.OFPMatch(
            ipv4_dst="10.0.0.2", 
            eth_dst="00:00:00:00:00:02",
            eth_type=0x0800,
            ip_proto=1,       # ICMP
            # ECHO REQUEST
            icmpv4_type=8,
        )
        actions2 = []
        self.add_flow(datapath, 2, match2, actions2)