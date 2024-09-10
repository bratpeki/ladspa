#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <ladspa.h>

#ifndef __cplusplus
# if __GNUC__
#  define ON_LOAD_ROUTINE   static void __attribute__ ((constructor)) init()
#  define ON_UNLOAD_ROUTINE static void __attribute__ ((destructor))  fini()
# else
#  define ON_LOAD_ROUTINE   void _init()
#  define ON_UNLOAD_ROUTINE void _fini()
# endif
#else
#endif

#define AMP_CONTROL 0
#define AMP_INPUT1 1
#define AMP_OUTPUT1 2
#define AMP_INPUT2 3
#define AMP_OUTPUT2 4

#define pow10f( x ) expf(2.302585092994046f*(x))

typedef struct {

	LADSPA_Data* pControl;
	LADSPA_Data* pInput1;
	LADSPA_Data* pOutput1;
	LADSPA_Data* pInput2;  /* Stereo */
	LADSPA_Data* pOutput2; /* Stereo */

} AmplifierDB;

static LADSPA_Handle instantiateAmplifierDB(
	const LADSPA_Descriptor* Descriptor,
	unsigned long SampleRate
) {
	return malloc(sizeof(AmplifierDB));
}

static void connectPortToAmplifierDB(
	LADSPA_Handle Instance,
	unsigned long Port,
	LADSPA_Data* DataLocation
) {

	AmplifierDB* sAmplifierDB;

	sAmplifierDB = (AmplifierDB*)Instance;

	switch (Port) {

		case AMP_CONTROL: sAmplifierDB->pControl = DataLocation; break;
		case AMP_INPUT1:  sAmplifierDB->pInput1  = DataLocation; break;
		case AMP_OUTPUT1: sAmplifierDB->pOutput1 = DataLocation; break;
		case AMP_INPUT2:  sAmplifierDB->pInput2  = DataLocation; break; /* Stereo */
		case AMP_OUTPUT2: sAmplifierDB->pOutput2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoAmplifierDB(
	LADSPA_Handle Instance,
	 unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	AmplifierDB* sAmplifierDB;
	unsigned long sampleIndex;

	sAmplifierDB = (AmplifierDB*)Instance;

	dInput = sAmplifierDB->pInput1;
	dOutput = sAmplifierDB->pOutput1;
	dGain = *(sAmplifierDB->pControl);

	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = dInput[sampleIndex] * pow10f(dGain / 20.0);

}

static void runStereoAmplifierDB(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	AmplifierDB* sAmplifierDB;
	unsigned long sampleIndex;

	sAmplifierDB = (AmplifierDB*)Instance;

	dGain = *(sAmplifierDB->pControl);

	dInput = sAmplifierDB->pInput1;
	dOutput = sAmplifierDB->pOutput1;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = dInput[sampleIndex] * pow10f(dGain / 20.0);

	dInput = sAmplifierDB->pInput2;
	dOutput = sAmplifierDB->pOutput2;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = dInput[sampleIndex] * pow10f(dGain / 20.0);

}

static void cleanupAmplifierDB( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* descmAmplifierDB = NULL;
LADSPA_Descriptor* descsAmplifierDB = NULL;

ON_LOAD_ROUTINE {

	char ** pnAmplifierDB;
	LADSPA_PortDescriptor* pdAmplifierDB;
	LADSPA_PortRangeHint* prhAmplifierDB;

	descmAmplifierDB = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	descsAmplifierDB = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (descmAmplifierDB) {

		descmAmplifierDB->UniqueID = 2001;

		descmAmplifierDB->Label = strdup("peki_ampdb_mono");
		descmAmplifierDB->Name = strdup("Peki's Mono dB Amplifier");
		descmAmplifierDB->Maker = strdup("Petar Katić (bratpeki)");
		descmAmplifierDB->Copyright = strdup("GPLv3");

		descmAmplifierDB->PortCount = 3;
		descmAmplifierDB->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdAmplifierDB = (LADSPA_PortDescriptor*)calloc(3, sizeof(LADSPA_PortDescriptor));
		pdAmplifierDB[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdAmplifierDB[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdAmplifierDB[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descmAmplifierDB->PortDescriptors = (const LADSPA_PortDescriptor*)pdAmplifierDB;

		pnAmplifierDB = (char **)calloc(3, sizeof(char*));
		pnAmplifierDB[AMP_CONTROL] = strdup("Gain");
		pnAmplifierDB[AMP_INPUT1] = strdup("Input");
		pnAmplifierDB[AMP_OUTPUT1] = strdup("Output");
		descmAmplifierDB->PortNames = (const char**)pnAmplifierDB;

		prhAmplifierDB = ((LADSPA_PortRangeHint*)calloc(3, sizeof(LADSPA_PortRangeHint)));
		prhAmplifierDB[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhAmplifierDB[AMP_CONTROL].LowerBound = -20;
		prhAmplifierDB[AMP_CONTROL].UpperBound = 20;
		prhAmplifierDB[AMP_INPUT1].HintDescriptor = 0;
		prhAmplifierDB[AMP_OUTPUT1].HintDescriptor = 0;
		descmAmplifierDB->PortRangeHints = (const LADSPA_PortRangeHint*)prhAmplifierDB;

		descmAmplifierDB->instantiate = instantiateAmplifierDB;
		descmAmplifierDB->connect_port = connectPortToAmplifierDB;
		descmAmplifierDB->activate = NULL;
		descmAmplifierDB->run = runMonoAmplifierDB;
		descmAmplifierDB->run_adding = NULL;
		descmAmplifierDB->set_run_adding_gain = NULL;
		descmAmplifierDB->deactivate = NULL;
		descmAmplifierDB->cleanup = cleanupAmplifierDB;
	}

	if (descsAmplifierDB) {

		descsAmplifierDB->UniqueID = 2002;

		descsAmplifierDB->Label = strdup("peki_ampdb_stereo");
		descsAmplifierDB->Name = strdup("Peki's Stereo dB Amplifier");
		descsAmplifierDB->Maker = strdup("Petar Katić (bratpeki)");
		descsAmplifierDB->Copyright = strdup("GPLv3");

		descsAmplifierDB->PortCount = 5;
		descsAmplifierDB->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdAmplifierDB = (LADSPA_PortDescriptor*)calloc(5, sizeof(LADSPA_PortDescriptor));
		pdAmplifierDB[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdAmplifierDB[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdAmplifierDB[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		pdAmplifierDB[AMP_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdAmplifierDB[AMP_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descsAmplifierDB->PortDescriptors = (const LADSPA_PortDescriptor*)pdAmplifierDB;

		pnAmplifierDB = (char **)calloc(5, sizeof(char*));
		pnAmplifierDB[AMP_CONTROL] = strdup("Gain");
		pnAmplifierDB[AMP_INPUT1] = strdup("Input (Left)");
		pnAmplifierDB[AMP_OUTPUT1] = strdup("Output (Left)");
		pnAmplifierDB[AMP_INPUT2] = strdup("Input (Right)");
		pnAmplifierDB[AMP_OUTPUT2] = strdup("Output (Right)");
		descsAmplifierDB->PortNames = (const char**)pnAmplifierDB;

		prhAmplifierDB = ((LADSPA_PortRangeHint*)calloc(5, sizeof(LADSPA_PortRangeHint)));
		prhAmplifierDB[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhAmplifierDB[AMP_CONTROL].LowerBound = -20;
		prhAmplifierDB[AMP_CONTROL].UpperBound = 20;
		prhAmplifierDB[AMP_INPUT1].HintDescriptor = 0;
		prhAmplifierDB[AMP_OUTPUT1].HintDescriptor = 0;
		prhAmplifierDB[AMP_INPUT2].HintDescriptor = 0;
		prhAmplifierDB[AMP_OUTPUT2].HintDescriptor = 0;
		descsAmplifierDB->PortRangeHints = (const LADSPA_PortRangeHint*)prhAmplifierDB;

		descsAmplifierDB->instantiate = instantiateAmplifierDB;
		descsAmplifierDB->connect_port = connectPortToAmplifierDB;
		descsAmplifierDB->activate = NULL;
		descsAmplifierDB->run = runStereoAmplifierDB;
		descsAmplifierDB->run_adding = NULL;
		descsAmplifierDB->set_run_adding_gain = NULL;
		descsAmplifierDB->deactivate = NULL;
		descsAmplifierDB->cleanup = cleanupAmplifierDB;

	}

}

static void deleteDescriptor(LADSPA_Descriptor* descAmplifierDB) {

	unsigned long lIndex;

	if (descAmplifierDB) {
		free((char*)descAmplifierDB->Label);
		free((char*)descAmplifierDB->Name);
		free((char*)descAmplifierDB->Maker);
		free((char*)descAmplifierDB->Copyright);
		free((LADSPA_PortDescriptor*)descAmplifierDB->PortDescriptors);
		for (lIndex = 0; lIndex < descAmplifierDB->PortCount; lIndex++)
			free((char*)(descAmplifierDB->PortNames[lIndex]));
		free((char**)descAmplifierDB->PortNames);
		free((LADSPA_PortRangeHint*)descAmplifierDB->PortRangeHints);
		free(descAmplifierDB);
	}

}

ON_UNLOAD_ROUTINE {
	deleteDescriptor(descmAmplifierDB);
	deleteDescriptor(descsAmplifierDB);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {

	switch (Index) {
		case 0: return descmAmplifierDB;
		case 1: return descsAmplifierDB;
		default: return NULL;
	}

}
