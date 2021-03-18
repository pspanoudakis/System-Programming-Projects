CC = g++

CXXFLAGS = -std=c++98
LDFLAGS = -lcrypto

TARGET = vaccineMonitor
OBJS = build/main.o build/app_utils.o build/parse_utils.o build/bloom_filter.o build/hash_functions.o build/hash_table.o build/linked_list.o build/rb_tree.o build/skip_list.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

build/main.o: app/main.cpp
	$(CC) -c app/main.cpp -o build/main.o

build/app_utils.o: app/app_utils.cpp
	$(CC) -c app/app_utils.cpp -o build/app_utils.o

build/parse_utils.o: app/parse_utils.cpp
	$(CC) -c app/parse_utils.cpp -o build/parse_utils.o

build/bloom_filter.o: lib/bloom_filter.cpp
	$(CC) -c lib/bloom_filter.cpp -o build/bloom_filter.o

build/hash_functions.o: lib/hash_functions.cpp
	$(CC) -c lib/hash_functions.cpp -o build/hash_functions.o

build/hash_table.o: lib/hash_table.cpp
	$(CC) -c lib/hash_table.cpp -o build/hash_table.o

build/linked_list.o: lib/linked_list.cpp
	$(CC) -c lib/linked_list.cpp -o build/linked_list.o

build/rb_tree.o: lib/rb_tree.cpp
	$(CC) -c lib/rb_tree.cpp -o build/rb_tree.o

build/skip_list.o: lib/skip_list.cpp
	$(CC) -c lib/skip_list.cpp -o build/skip_list.o

clean:
	rm $(TARGET) $(OBJS)
