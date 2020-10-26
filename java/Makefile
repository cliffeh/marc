SRC=$(wildcard src/marc/*.java)
OBJ=$(patsubst src/marc/%.java, build/marc/%.class, $(SRC))

all: $(OBJ)

$(OBJ): build $(SRC)
	javac -d build $(SRC)

build:
	mkdir -p build

clean:
	echo "nothing to clean..."

realclean: clean
	rm -rf build
