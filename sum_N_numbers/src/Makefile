# Variabili
CC = mpicc
CFLAGS = -Wall -g
LDFLAGS = -lm  # Aggiungi l'opzione -lm per collegare la libreria matematica

# Nome dell'eseguibile
TARGET = my_mpi_program

# Lista dei file oggetto
OBJECTS = Main.o Strategy.o Utils.o

# Regola predefinita: compilare il programma MPI
all: $(TARGET)

# Regola per la creazione dell'eseguibile
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# Regola per la compilazione di main.c
main.o: main.c
	$(CC) $(CFLAGS) -c Main.c -o Main.o

# Regola per la compilazione di Strategy.c
Strategy.o: Strategy.c
	$(CC) $(CFLAGS) -c Strategy.c -o Strategy.o

# Regola per la compilazione di Strategy.c
Utils.o: Utils.c
	$(CC) $(CFLAGS) -c Utils.c -o Utils.o

# Pulizia dei file oggetto e dell'eseguibile
clean:
	rm -f $(OBJECTS) $(TARGET)