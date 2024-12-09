#ifndef MENUINDEX_H
#define MENUINDEX_H

#define TRX1    "Node1 "
#define TRX2    "Node2 "
#define STB   "S:"
#define MAIN  "M:"
#define RECEIVE "R:"
#define INCALL1 "CallIn1:"

#define CONNECTED       "Connected "
#define DISCONNECTED    "Disconnected"
#define CONNECTING      "Connecting"
#define RECONNECTING    "Reconnecting"
#define CALLING         "Calling"
#define CONNSTB         "Standby"

#define PTTON           "Tx"
#define RXON            "Rx"
#define PTTRX           "TR"
#define STANDBY         "--"

#define HOMEDISPALY 0

#define strHomeLine1                "Node 1:   "
#define strHomeLine2                "Node 2:   "

#define MAINMENU_NODE_CONFIG    1
#define MAINMENU_NETWORK        2
#define MAINMENU_AUDIO          3
#define MAINMENU_PORT_INTERFACE 4
#define MAINMENU_SYSINFO        5


#define strMainmenu                 "      Main Menu     "

#define strMainNodeConfig           " Node Configuration "
#define strMainNetwork              "    Local Network   "
#define strMainAudio                "   Audio I/O Level  "
#define strMainPortInterface        "   Port Interface   "
#define strMainSystemInfo           "     System Info    "

#define NODECONFIG1             11
#define NODECONFIG2             12
#define NODECONFIG              13

#define strNodeConfig1              "       Node 1       "
#define strNodeConfig2              "       Node 2       "

#define strNodeConfig               "       Node  "


#define NODE1_NODETYPE           110
#define NODE1_ADDRESS            111
#define NODE1_ACTIVE             112
#define NODE1_RECONN             113
#define NODE1_STATUS             114
#define NODE1_DURATION           115

#define NODE2_NODETYPE           120
#define NODE2_ADDRESS            121
#define NODE2_RECONN             122
#define NODE2_ACTIVE             123
#define NODE2_STATUS             124
#define NODE2_DURATION           125

#define NODE_ADDRESS            130
#define NODE_ACTIVE             131
#define NODE_RECONN             132
#define NODE_STATUS             133
#define NODE_DURATION           134


#define strTxRxNodeURI              "      TxRx Node     "
#define strRxNodeURI                "       Rx Node      "
#define strTxNodeURI                "       Tx Node      "
#define strNodeAddr                 "    Node Address    "
#define strNodeReconnect            "      Reconnect     "
#define strNodeDisconnect           "      Disconnect    "
#define strNodeActive               "     Node Active    "
#define strNodeStatus               "  Connection Status "
#define strNodeDuration             "Connection Duration "

#define NODE1_SHOW_ADDR          1100
#define NODE2_SHOW_ADDR          1200
#define NODE_SHOW_ADDR           1300
#define NODE1_ACTIVE_ENABLE     1110
#define NODE1_ACTIVE_DISABLE    1111

#define NODE2_ACTIVE_ENABLE     1210
#define NODE2_ACTIVE_DISABLE    1211
#define strNodeEnable               " [Enable]  Disable  "
#define strNodeDisable              "  Enable  [Disable] "

#define NODE1_RECONN_START      1121
#define NODE2_RECONN_START      1221

#define strNodeReconnectStart       " Enter to Reconnect "

#define NODE1_RECONNECTING      11210
#define NODE2_RECONNECTING      12210

#define NODE1_CONN_STATUS       1130
#define NODE2_CONN_STATUS       1230

#define NODE1_CONN_DURATION     1140
#define NODE2_CONN_DURATION     1240
#define NODE_CONN_DURATION      1340
#define strNetwork_eth            "   Phy-Network ID   "
#define strNetwork_eth0           "        ETH0        "
#define strNetwork_eth1           "        ETH1        "
#define strNetwork_eth0_IP        "ETH0 IP Address     "
#define strNetwork_eth0_SUBNET    "ETH0 Subnet Mask    "
#define strNetwork_eth0_GW        "ETH0 Gateway        "
#define strNetwork_eth0_failed    "ETH0 No PHY found   "
#define strNetwork_eth0_priDNS    "ETH0 DNS1           "
#define strNetwork_eth0_secDNS    "ETH0 DNS2           "
#define strNetwork_eth1_IP        "ETH1 IP Address     "
#define strNetwork_eth1_SUBNET    "ETH1 Subnet Mask    "
#define strNetwork_eth1_GW        "ETH1 Gateway        "
#define strNetwork_eth1_failed    "ETH1 No PHY found   "
#define strNetwork_eth1_priDNS    "ETH1 DNS1           "
#define strNetwork_eth1_secDNS    "ETH1 DNS2           "
#define strNetwork_RESTART        "   Restart Network  "
#define strNetwork_eth0_MAC       "ETH0 MAC Address    "
#define strNetwork_eth1_MAC       "ETH1 MAC Address    "



#define NETWORK_ETH_PHY          20
#define RESTART_NETWORK          21
#define NETWORK_RESTART          22

#define NETWORK_ETH0             200
#define NETWORK_ETH1             201

#define RESTARTING_NETWORK       221

#define NETWORK_ETH0_0_IP             2000
#define NETWORK_ETH0_1_SUBNET         2001
#define NETWORK_ETH0_2_GW             2002
#define NETWORK_ETH0_3_DNS            2003
#define NETWORK_ETH0_4_MAC            2004
#define NETWORK_ETH0_5_NOTFOUND       2005


#define ETH0_IP_SHOW                  20000
#define ETH0_SUBNET_SHOW              20010
#define ETH0_GW_SHOW                  20020
#define ETH0_DNS_SHOW                 20030
#define ETH0_MAC_SHOW                 20040

#define NETWORK_ETH1_0_IP             2010
#define NETWORK_ETH1_1_SUBNET         2011
#define NETWORK_ETH1_2_GW             2012
#define NETWORK_ETH1_3_DNS            2013
#define NETWORK_ETH1_4_MAC            2014
#define NETWORK_ETH1_5_NOTFOUND       2015

#define ETH1_IP_SHOW                  20100
#define ETH1_SUBNET_SHOW              20110
#define ETH1_GW_SHOW                  20120
#define ETH1_DNS_SHOW                 20130
#define ETH1_MAC_SHOW                 20140

#define strNetwork_RESTARTING     "     Restarting     "
#define strNetwork_EDITLAN        "Edit LAN Connection "
#define strNetwork_RESETING       "Reconnect........   "
#define strNetwork_RESET          "Enter to Reset      "



//#define SETNETWORK_0_SETDHCP        260
//#define SETNETWORK_1_SETIP          261
//#define SETNETWORK_2_SETSUBNET      262
//#define SETNETWORK_3_SETGW          263
//#define SETNETWORK_4_SETDNS1        264
//#define SETNETWORK_5_SETDNS2        265
//#define SETNETWORK_6_SAVE           266
//#define SETNETWORK_7_RESETING       267

//#define DHCP_NETWORK_0_DHCP         2600
//#define DHSP_NETWORK_1_STATIC       2601

//#define strNetwork_DHCP             "        DHCP        "
//#define strNetwork_STATIC           "       Static       "

//#define strNetwork_SETDHCP          "Setting up DHCP     "
//#define strNetwork_SETIP            "Setting up IP Addr. "
//#define strNetwork_SETSUBNET        "Setting up Subnet   "
//#define strNetwork_SETGW            "Setting up Gateway  "
//#define strNetwork_SETDNS1          "Setting up Pri. DNS "
//#define strNetwork_SETDNS2          "Setting up Sec. DNS "
//#define strNetwork_SAVE             "Save&Reset Network  "
//#define strNetwork_RESETING         "Reconnect........   "
//#define strNetwork_RESET            "Enter to Reset      "

#define AUDIO0_IN_LEVEL             30
#define AUDIO1_OUT_LEVEL            31
#define AUDIO2_I_O_LEVEL            32

#define strAudioInLevel             " Audio Input Level  "
#define strAudioOutLevel            " Audio Output Level "
#define strAudioInOutVU             "      VU Meter      "


#define SETAUDIO_IN_LEVEL           300
#define SETAUDIO_OUT_LEVEL          310
#define SETAUDIO_VU_METER           311

#define PORT0_RJ45                  40
#define PORT1_DB9                   41

#define strPortRJ45                 "  [RJ-45]    DB-9   "
#define strPortDB9                  "   RJ-45    [DB-9]  "

#define SETPORT_RJ45                400
#define SETPORT_DB9                 410

#define SYSTEM_BACKLIGHT            50
#define SYSTEM_UPTIME               51
#define SYSTEM_SWV                  52
#define SYSTEM_UPDATE               53

#define BACKLIGHT_SETUP             500
#define UPTIME                      510
#define SOFTWAREVERSION             520
#define SOFTWAREUPDATE              530

#define SOFTWAREUPDATE_SCANFILE     5300
#define SOFTWAREUPDATE_PRSENTER     5301
#define SOFTWAREUPDATE_UPDATEING    5302
#define SOFTWAREUPDATE_UPDATED      5303
#define SOFTWAREUPDATE_FILEMISS     5304

#define strUptime                   "      Uptime       "
#define strSystemBacklight          "   LCD Backlight   "
#define strSystemSwVersion          "  Software Version "
#define strSystemSwUpdate           "      Update       "

#define strUpdating                 "Updating, Pls wait "
#define strUpdateed                 "Update completed   "
#define strUpdateMissFile           "Find missing files "
#define strUpdateScanfile           "Scanning file      "
#define strUpdateEnter              "Enter to Update    "

#endif // MENUINDEX_H

