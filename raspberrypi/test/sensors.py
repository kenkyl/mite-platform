import re

class SensorMessage:
    def __init__(self, raw_message):
        self.valid = True 
        self.parseMessage(raw_message)

    def parseMessage(self, raw_message):
        match = re.search(r'([\w]+)\:\:([\w]+)\:\:([\w\s]+)\:\:([\w\-\.\,]+)',
                          raw_message)
        if match:
            #print match.group() 
            self.id = match.group(1)
            #print match.group(1) 
            self.type = match.group(2)
            #print match.group(2)
            self.label = match.group(3)
            #print match.group(3)
            self.value = match.group(4)
            #print match.group(4)
        else:
            self.valid = False
            print 'invalid message' 

    
