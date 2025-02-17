#ifndef HC06_BT_H
#define HC06_BT_H

#include <iostream>
#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <unistd.h>

using namespace std;

class HC06Bluetooth {
private:
    string mac_address;
    int s;
public:
    HC06Bluetooth(const string& macAddress) : mac_address(macAddress), s(-1) {}

    ~HC06Bluetooth() {
        if (s != -1) {
            close(s);
        }
    }

    bool connectToDevice() {
        struct sockaddr_rc addr = { 0 };

        s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
        if (s == -1) {
            cerr << "Failed to create socket" << endl;
            return false;
        }

        addr.rc_family = AF_BLUETOOTH;
        addr.rc_channel = (uint8_t)1;
        str2ba(mac_address.c_str(), &addr.rc_bdaddr);

        int status = connect(s, (struct sockaddr*)&addr, sizeof(addr));
        if (status == 0) {
            cout << "Connected to HC-06" << endl;
            return true;
        }
        else {
            cerr << "Failed to connect to HC-06" << endl;
            return false;
        }
    }

    double getData() {
        if (s == -1) {
            cerr << "Socket not connected" << endl;
            return 0.0;
        }

        char buf[1024] = { 0 };
        int bytes_read = read(s, buf, sizeof(buf));
        if (bytes_read > 0) {
            try {
                return stod(buf);
            }
            catch (const invalid_argument& e) {
                cerr << "Received invalid data: " << buf << endl;
            }
            catch (const out_of_range& e) {
                cerr << "Received out of range data: " << buf << endl;
            }
        }
        else
            cerr << "Failed to read data from HC-06" << endl;

        return 0.0;
    }
};

#endif // HC06_BT_H