# Variabili
CC = mpicc
CFLAGS = -Wall -g

# Nome dell'eseguibile
TARGET = my_mpi_program

# Lista dei file oggetto
OBJECTS = main.o Strategy.o

# Regola predefinita: compilare il programma MPI
all: $(TARGET)

# Regola per la creazione dell'eseguibile
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Regola per la compilazione di main.c
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

# Regola per la compilazione di Strategy.c
Strategy.o: Strategy.c
	$(CC) $(CFLAGS) -c Strategy.c -o Strategy.o

# Pulizia dei file oggetto e dell'eseguibile
clean:
	rm -f $(OBJECTS) $(TARGET)
