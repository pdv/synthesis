#ifndef __SYNTHESIS__
#define __SYNTHESIS__
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "IPlug_include_in_plug_hdr.h"
#include "EnvelopeGenerator.h"
#include "Filter.h"

class Synthesis : public IPlug
{
public:
  Synthesis(IPlugInstanceInfo instanceInfo);
  ~Synthesis();
  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  // to receive MIDI messages:
  void ProcessMidiMsg(IMidiMsg* pMsg);

  // GUI Keyboard:
  // Return non-zero if one or more keys are playing.
  inline int GetNumKeys() const { return mMIDIReceiver.getNumKeys(); };
  // Return true if the specified key is playing
  inline bool GetKeyStatus(int key) const { return mMIDIReceiver.getKeyStatus(key); };
  static const int virtualKeyboardMinimumNoteNumber = 48;
  int lastVirtualKeyboardNoteNumber;

private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
  IControl* mVirtualKeyboard;
  EnvelopeGenerator mEnvelopeGenerator;
  EnvelopeGenerator mFilterEnvelopeGenerator;
  Filter mFilter;
  double filterEnvelopeAmount;
  void processVirtualKeyboard();
  inline void onBeganEnvelopeCycle() { mOscillator.setMuted(false); }
  inline void onFinishedEnvelopeCycle() { mOscillator.setMuted(true); }
  inline void onNoteOn(const int noteNumber, const int velocity) {
	  mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
	  mFilterEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
  };
  inline void onNoteOff(const int noteNumber, const int velocity) {
	  mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
	  mFilterEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
  };
  Oscillator mLFO;
  double lfoFilterModAmount;

};

#endif
