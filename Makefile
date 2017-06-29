all: idevicecontrolproxy

SOURCE=src


idevicecontrolproxy: $(SOURCE)/whitelist.cpp $(SOURCE)/cmdp.cpp $(SOURCE)/resp.cpp $(SOURCE)/main.cpp 
	g++ -g -Wall `pkg-config libwebsockets --libs --cflags` jsmn/libjsmn.a $^ -o $@

clean:
	rm -f idevicecontrolproxy
	rm -rf idevicecontrolproxy.dSYM
