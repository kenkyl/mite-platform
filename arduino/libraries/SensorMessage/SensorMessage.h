#ifndef SensorMessage_H
#define SensorMessage_H

class SensorMessage {
    char id[8];
    char label[16]; 
    char type[8]; 
    char value[16];
  public:
    SensorMessage ();
    SensorMessage (char*, char*, char*, char*); 
    void setId (char*); 
    void setLabel (char*); 
    void setType (char*); 
    void setValue (char*); 
    void parseIncoming (char*); 
};

#endif

