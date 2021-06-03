CC = g++

CXXFLAGS = -Wall -Wno-sign-compare -std=c++11

MONITOR_OBJS = build/monitor.o \
build/app_utils.o \
build/parse_utils.o \
build/bloom_filter.o \
build/hash_functions.o \
build/hash_table.o \
build/linked_list.o \
build/rb_tree.o \
build/skip_list.o \
build/messaging.o \
build/sem_utils.o
PARENT_OBJS = build/parent_monitor.o build/parent_monitor_utils.o build/app_utils.o build/parse_utils.o build/bloom_filter.o build/hash_functions.o build/hash_table.o build/linked_list.o build/rb_tree.o build/skip_list.o build/messaging.o

all: monitorServer travelMonitorClient

monitorServer: $(MONITOR_OBJS)
	$(CC) $(CXXFLAGS) -o monitorServer $(MONITOR_OBJS) -lcrypto -lpthread

travelMonitorClient: $(PARENT_OBJS)
	$(CC) $(CXXFLAGS) -o travelMonitorClient $(PARENT_OBJS) -lcrypto

build/parent_monitor.o: app/parent_monitor.cpp
	$(CC) -c $(CXXFLAGS) app/parent_monitor.cpp -o build/parent_monitor.o

build/parent_monitor_utils.o: app/parent_monitor_utils.cpp
	$(CC) -c $(CXXFLAGS) app/parent_monitor_utils.cpp -o build/parent_monitor_utils.o

build/monitor.o: app/monitor.cpp
	$(CC) -c $(CXXFLAGS) app/monitor.cpp -o build/monitor.o

build/app_utils.o: app/app_utils.cpp
	$(CC) -c $(CXXFLAGS) app/app_utils.cpp -o build/app_utils.o

build/parse_utils.o: app/parse_utils.cpp
	$(CC) -c $(CXXFLAGS) app/parse_utils.cpp -o build/parse_utils.o

build/bloom_filter.o: lib/bloom_filter.cpp
	$(CC) -c $(CXXFLAGS) lib/bloom_filter.cpp -o build/bloom_filter.o

build/hash_functions.o: lib/hash_functions.cpp
	$(CC) -c $(CXXFLAGS) lib/hash_functions.cpp -o build/hash_functions.o

build/hash_table.o: lib/hash_table.cpp
	$(CC) -c $(CXXFLAGS) lib/hash_table.cpp -o build/hash_table.o

build/linked_list.o: lib/linked_list.cpp
	$(CC) -c $(CXXFLAGS) lib/linked_list.cpp -o build/linked_list.o

build/rb_tree.o: lib/rb_tree.cpp
	$(CC) -c $(CXXFLAGS) lib/rb_tree.cpp -o build/rb_tree.o

build/skip_list.o: lib/skip_list.cpp
	$(CC) -c $(CXXFLAGS) lib/skip_list.cpp -o build/skip_list.o

build/messaging.o: lib/messaging.cpp
	$(CC) -c $(CXXFLAGS) lib/messaging.cpp -o build/messaging.o

build/sem_utils.o: app/sem_utils.cpp
	$(CC) -c $(CXXFLAGS) app/sem_utils.cpp -o build/sem_utils.o

clean:
	rm monitorServer travelMonitorClient build/parent_monitor.o build/parent_monitor_utils.o $(MONITOR_OBJS)
