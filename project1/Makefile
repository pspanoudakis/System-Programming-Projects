CC = g++

CXXFLAGS = -Wall -std=c++98

TARGET = vaccineMonitor
OBJS = build/main.o build/app_utils.o build/parse_utils.o build/bloom_filter.o build/hash_functions.o build/hash_table.o build/linked_list.o build/rb_tree.o build/skip_list.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lcrypto

build/main.o: app/main.cpp
	$(CC) -c $(CXXFLAGS) app/main.cpp -o build/main.o

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

clean:
	rm $(TARGET) $(OBJS)
