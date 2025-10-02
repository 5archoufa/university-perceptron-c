#include "state_machine/instances/sm_perceptron/sm_perceptron.h"
#include "state_machine/instances/sm_perceptron/states/s_neural_network.h"
#include "state_machine/state_machine.h"

static StateMachine *sm_perceptron = NULL;
static StateMachine_State *s_neuralNetwork = NULL;

StateMachine* SMPerceptron_Init()
{
    sm_perceptron = StateMachine_Create("SMPerceptron");
    s_neuralNetwork = SMPerceptron_SNeuralNetwork_Create();
    StateMachine_TrySetState(sm_perceptron, s_neuralNetwork, 0, NULL);
    return sm_perceptron;
}

void SMPerceptron_Free()
{
    if (s_neuralNetwork != NULL)
    {
        SMPerceptron_SNeuralNetwork_Free(s_neuralNetwork);
    }
    if (sm_perceptron != NULL)
    {
        StateMachine_Free(sm_perceptron);
    }
}