#include <math.h>

enum OscillatorMode {
	OSCILLATOR_MODE_SINE,
	OSCILLATOR_MODE_SAW,
	OSCILLATOR_MODE_SQUARE,
	OSCILLATOR_MODE_TRIANGLE,
	kNumOscillatorModes
};

class Oscillator {
private:
	OscillatorMode mOscillatorMode;
	const double mPI;
	double mFrequency;
	double mPhase;
	double mSampleRate;
	double mPhaseIncrement;
	void updateIncrement();
	const double twoPI;
	bool isMuted;
public:
	void setMode(OscillatorMode mode);
	void setFrequency(double frequency);
	void setSampleRate(double sampleRate);
	void generate(double* buffer, int nFrames);
	Oscillator() :
		mOscillatorMode(OSCILLATOR_MODE_SINE),
		mPI(2 * acos(0.0)),
		twoPI(2 * mPI),
		isMuted(true),
		mFrequency(440.0),
		mPhase(0.0),
		mSampleRate(44100.0) { updateIncrement(); };
	inline void setMuted(bool muted) { isMuted = muted; }
	double nextSample();
};