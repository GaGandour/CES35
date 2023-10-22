from mininet.cli import CLI
from mininet.link import TCLink
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.topo import Topo
from mininet.node import Controller
from mininet.log import setLogLevel, info
from mn_wifi.node import Station, AP
from mn_wifi.cli import CLI
from mn_wifi.net import Mininet_wifi


class CustomTopology(Topo):
    def build(self):
        # Add access points
        ap1 = self.addAccessPoint('ap1', ssid='my-ssid', mode='g', channel='1', position='10,20,0')
        
        # Add stations
        sta1 = self.addStation('sta1', position='0,10,0')
        sta2 = self.addStation('sta2', position='20,10,0')
        
        # Add links between stations and access points
        self.addLink(sta1, ap1)
        self.addLink(sta2, ap1)


def create_mininet_wifi_network():
    topo = CustomTopology()
    net = Mininet_wifi(topo=topo, controller=Controller)
    net.start()
    return net


def run_ping_test(net):
    sta1 = net.get('sta1')
    sta2 = net.get('sta2')
    result = sta1.cmd('ping -c 5 ' + sta2.IP())
    print(result)


if __name__ == '__main__':
    setLogLevel('info')
    net = create_mininet_wifi_network()
    run_ping_test(net)
    CLI(net)
    net.stop()

