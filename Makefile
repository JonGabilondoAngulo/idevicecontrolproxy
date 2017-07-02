all: idevicecontrolproxy

SOURCE=src


idevicecontrolproxy: $(SOURCE)/json_ext.cpp $(SOURCE)/whitelist.cpp $(SOURCE)/cmd.cpp $(SOURCE)/resp.cpp $(SOURCE)/main.cpp 
	g++ -g -Wall `pkg-config libwebsockets --libs --cflags` jsmn/libjsmn.a $^ -o $@

clean:
	rm -f idevicecontrolproxy
	rm -rf idevicecontrolproxy.dSYM
