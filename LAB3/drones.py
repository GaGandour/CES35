from mininet.log import info, setLogLevel
from mn_wifi.cli import CLI
from mn_wifi.link import adhoc, wmediumd
from mn_wifi.net import Mininet_wifi
from mn_wifi.wmediumdConnector import interference

INITIAL_POS_1 = '0.0,5.0,0.0'
INITIAL_POS_2 = '0.0,10.0,0.0'
FINAL_POS_1 = '10.0,5.0,0.0'
FINAL_POS_2 = '10.0,10.0,0.0'

DRONE_RANGE = 10

def topology():
    "Create a network."
    net = Mininet_wifi(link=wmediumd, wmediumd_mode=interference)

    # Criacao dos drones
    info("*** Creating drones\n")
    drone1 = net.addStation('drone1', ip6='fe80::1', range=DRONE_RANGE)
    drone2 = net.addStation('drone2', ip6='fe80::2', range=DRONE_RANGE)

    # Define um modelo de propagacao de sinal
    net.setPropagationModel(model="logDistance", exp=4)

    info("*** Configuring nodes\n")
    net.configureNodes()

    info("*** Creating links\n")
    net.addLink(
        drone1, 
        cls=adhoc, 
        intf='drone1-wlan0',
        ssid='adhocNet', 
        mode='g', 
        channel=5,
        ht_cap='HT40+',
    )
    net.addLink(
        drone2, 
        cls=adhoc, 
        intf='drone2-wlan0',
        ssid='adhocNet', 
        mode='g', 
        channel=5,
    )

    info('*** Plotting Graph\n')
    net.plotGraph(
        min_x=-10, 
        max_x=30,
        min_y=-10,
        max_y=30,
    )
    
    # Movimento dos drones 
    p1 = {'position': INITIAL_POS_1}
    p2 = {'position': INITIAL_POS_2}
    p3 = {'position': FINAL_POS_1}
    p4 = {'position': FINAL_POS_2}
    
    net.startMobility(time=0, mob_rep=10, reverse=False)

    net.mobility(drone1, 'start', time=1, **p1)
    net.mobility(drone2, 'start', time=1, **p2)
    net.mobility(drone1, 'stop', time=12, **p3)
    net.mobility(drone2, 'stop', time=12, **p4)
    
    net.stopMobility(time=12)


    info("*** Starting network\n")
    net.build()

    info("\n*** Addressing...\n")
    drone1.setIP6('2001::1/64', intf="drone1-wlan0")
    drone2.setIP6('2001::2/64', intf="drone2-wlan0")

    info("*** Running CLI\n")
    CLI(net)

    info("*** Stopping network\n")
    net.stop()


if __name__ == '__main__':
    setLogLevel('info')
    topology()
