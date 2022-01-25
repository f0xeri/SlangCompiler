//
// Created by Yaroslav on 03.10.2021.
//

#ifndef SLANGPARSER_LOGGER_HPP
#define SLANGPARSER_LOGGER_HPP

/*using namespace std;

enum typelog {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

struct structlog {
    bool headers = false;
    typelog level = WARN;
};


class LOG {
public:
    structlog LOGCFG = {true, DEBUG};
    LOG() {}
    LOG(typelog type) {
        msglevel = type;
        if(LOGCFG.headers) {
            operator << ("[" + getLabel(type) + "] ");
        }
    }
    ~LOG() {
        if(opened) {
            cout << endl;
        }
        opened = false;
    }
    template<class T>
    LOG &operator<<(const T &msg) {
        if(msglevel >= LOGCFG.level) {
            cout << msg;
            opened = true;
        }
        return *this;
    }
private:
    bool opened = false;
    typelog msglevel = DEBUG;
    inline string getLabel(typelog type) {
        string label;
        switch(type) {
            case DEBUG: label = "DEBUG"; break;
            case INFO:  label = "INFO"; break;
            case WARN:  label = "WARN"; break;
            case ERROR: label = "ERROR"; break;
        }
        return label;
    }
};*/

#endif //SLANGPARSER_LOGGER_HPP
