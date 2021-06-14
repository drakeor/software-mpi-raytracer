# Since this project is a bit bigger, a Makefile will work better
# I use C99 standard in case this runs on scholar
# Also because it needs to run on scholar, this requires nothing outside
# standard libs
CC = gcc
CC_MPI = mpicc
CFLAGS = -I. -std=c99 -g
MPIFLAGS = -I. -std=c99 -g
OBJS = main.o render_bmp.o raytracer.o linmath_ext.o scene.o
LIBS = -lm -fopenmp

# folders to store stuff
BIN_DIR = bin
OBJ_DIR = obj

# Use the obj/ directory
OBJ_WITH_DIR = $(patsubst %,$(OBJ_DIR)/%,$(OBJS))
OBJ_WITH_DIR_OPENMP2 = $(patsubst %,$(OBJ_DIR)/%.openmp2,$(OBJS))
OBJ_WITH_DIR_OPENMP4 = $(patsubst %,$(OBJ_DIR)/%.openmp4,$(OBJS))
OBJ_WITH_DIR_OPENMP8 = $(patsubst %,$(OBJ_DIR)/%.openmp8,$(OBJS))
OBJ_WITH_DIR_OPENMP16 = $(patsubst %,$(OBJ_DIR)/%.openmp16,$(OBJS))
OBJ_WITH_DIR_MPI = $(patsubst %,$(OBJ_DIR)/%.mpi,$(OBJS))

# Compile stuff into the obj/ folder
$(OBJ_DIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) -c -o $@.openmp2 $< $(CFLAGS) -fopenmp -D USE_OPENMP=1 -D OPENMP_THREAD_AMOUNT=2
	$(CC) -c -o $@.openmp4 $< $(CFLAGS) -fopenmp -D USE_OPENMP=1 -D OPENMP_THREAD_AMOUNT=4
	$(CC) -c -o $@.openmp8 $< $(CFLAGS) -fopenmp -D USE_OPENMP=1 -D OPENMP_THREAD_AMOUNT=8
	$(CC) -c -o $@.openmp16 $< $(CFLAGS) -fopenmp -D USE_OPENMP=1 -D OPENMP_THREAD_AMOUNT=16
	$(CC_MPI) -c -o $@.mpi $< $(MPIFLAGS) -D USE_MPI=1

# Compile the raytracer
all: raytracer raytracer_openmp2 raytracer_openmp4 raytracer_openmp8 raytracer_openmp16 raytracer_mpi

raytracer: $(OBJ_WITH_DIR)
	$(CC) -o $(BIN_DIR)/raytracer $^ $(CFLAGS) $(LIBS)

raytracer_openmp2: $(OBJ_WITH_DIR_OPENMP2)
	$(CC) -o $(BIN_DIR)/raytracer_openmp2 $^ $(CFLAGS) $(LIBS)

raytracer_openmp4: $(OBJ_WITH_DIR_OPENMP4)
	$(CC) -o $(BIN_DIR)/raytracer_openmp4 $^ $(CFLAGS) $(LIBS)

raytracer_openmp8: $(OBJ_WITH_DIR_OPENMP8)
	$(CC) -o $(BIN_DIR)/raytracer_openmp8 $^ $(CFLAGS) $(LIBS)

raytracer_openmp16: $(OBJ_WITH_DIR_OPENMP16)
	$(CC) -o $(BIN_DIR)/raytracer_openmp16 $^ $(CFLAGS) $(LIBS)

raytracer_mpi: $(OBJ_WITH_DIR_MPI)
	$(CC_MPI) -o $(BIN_DIR)/raytracer_mpi $^ $(CFLAGS) $(LIBS)

# Clean everything
clean:
	rm -f $(OBJ_DIR)/*
	rm -f $(BIN_DIR)/*
	echo "placeholder" > $(BIN_DIR)/placeholder.txt
	echo "placeholder" > $(OBJ_DIR)/placeholder.txt