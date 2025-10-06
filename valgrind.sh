make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./build/perceptron
# valgrind --suppressions=valgrind-gl.supp --leak-check=full ./build/perceptron