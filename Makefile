CC = g++

CXXFLAGS = -std=c++98
LDFLAGS = -lcrypto

TARGET = vaccineMonitor
OBJS = build/main.o build/app_utils.o build/parse_utils.o build/bloom_filter.o build/hash_functions.o build/hash_table.o build/linked_list.o build/rb_tree.o build/skip_list.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJS):
	$(CC) -c app/main.cpp -o build/main.o
	$(CC) -c app/app_utils.cpp -o build/app_utils.o
	$(CC) -c app/parse_utils.cpp -o build/parse_utils.o
	$(CC) -c lib/bloom_filter.cpp -o build/bloom_filter.o
	$(CC) -c lib/hash_functions.cpp -o build/hash_functions.o
	$(CC) -c lib/hash_table.cpp -o build/hash_table.o
	$(CC) -c lib/linked_list.cpp -o build/linked_list.o
	$(CC) -c lib/rb_tree.cpp -o build/rb_tree.o
	$(CC) -c lib/skip_list.cpp -o build/skip_list.o

clean:
	rm $(TARGET) $(OBJS)
