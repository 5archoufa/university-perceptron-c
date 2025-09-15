
typedef struct Neuron
{
    float *weights;
    int weightCount;
    float bias;
} Neuron;

Neuron *CreateNeuron(int weightCount, float *weights, int bias);
Neuron *CreateNeuron_RandomWeights(int weightCount);
void FreeNeuron(Neuron *neuron);
void PrintNeuron(Neuron *neuron);