#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"

const int kNumPrograms = 3;

enum EParams
{
	mWaveform = 0,
	mAttack,
	mDecay,
	mSustain,
	mRelease,
	mFilterMode,
	mFilterCutoff,
	mFilterResonance,
	mFilterAttack,
	mFilterDecay,
	mFilterSustain,
	mFilterRelease,
	mFilterEnvelopeAmount,
	kNumParams
};

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT,
	kKeybX = 1,
	kKeybY = 230
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.),
  lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1),
  filterEnvelopeAmount(0.0), lfoFilterModAmount(0.1) {
  TRACE;
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);

  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

  //                            C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);

  pGraphics->AttachControl(mVirtualKeyboard);

  // Waveform switch
  GetParam(mWaveform)->InitEnum("Waveform", OSCILLATOR_MODE_SINE, kNumOscillatorModes);
  GetParam(mWaveform)->SetDisplayText(0, "Sine");
  IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
  pGraphics->AttachControl(new ISwitchControl(this, 24, 38, mWaveform, &waveformBitmap));

  // Knob bitmap for ADSR
  IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 64);
  // Attack knob:
  GetParam(mAttack)->InitDouble("Attack", 0.01, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 95, 34, mAttack, &knobBitmap));
  // Decay knob:
  GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 177, 34, mDecay, &knobBitmap));
  // Sustain knob:
  GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 259, 34, mSustain, &knobBitmap));
  // Release knob:
  GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 341, 34, mRelease, &knobBitmap));

  // Filter mode
  GetParam(mFilterMode)->InitEnum("Filter Mode", Filter::FILTER_MODE_LOWPASS, Filter::kNumFilterModes);
  IBitmap filtermodeBitmap = pGraphics->LoadIBitmap(FILTERMODE_ID, FILTERMODE_FN, 3);
  pGraphics->AttachControl(new ISwitchControl(this, 24, 123, mFilterMode, &filtermodeBitmap));

  // Knobs for filter cutoff and resonance
  IBitmap smallKnobBitmap = pGraphics->LoadIBitmap(KNOB_SMALL_ID, KNOB_SMALL_FN, 64);
  // Cutoff knob:
  GetParam(mFilterCutoff)->InitDouble("Cutoff", 0.99, 0.01, 0.99, 0.001);
  GetParam(mFilterCutoff)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 5, 177, mFilterCutoff, &smallKnobBitmap));
  // Resonance knob:
  GetParam(mFilterResonance)->InitDouble("Resonance", 0.01, 0.01, 1.0, 0.001);
  pGraphics->AttachControl(new IKnobMultiControl(this, 61, 177, mFilterResonance, &smallKnobBitmap));

  // Knobs for filter envelope
  // Attack knob
  GetParam(mFilterAttack)->InitDouble("Filter Env Attack", 0.01, 0.01, 10.0, 0.001);
  GetParam(mFilterAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 139, 178, mFilterAttack, &smallKnobBitmap));
  // Decay knob:
  GetParam(mFilterDecay)->InitDouble("Filter Env Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mFilterDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 195, 178, mFilterDecay, &smallKnobBitmap));
  // Sustain knob:
  GetParam(mFilterSustain)->InitDouble("Filter Env Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mFilterSustain)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 251, 178, mFilterSustain, &smallKnobBitmap));
  // Release knob:
  GetParam(mFilterRelease)->InitDouble("Filter Env Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mFilterRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 307, 178, mFilterRelease, &smallKnobBitmap));

  // Filter envelope amount knob:
  GetParam(mFilterEnvelopeAmount)->InitDouble("Filter Env Amount", 0.0, -1.0, 1.0, 0.001);
  pGraphics->AttachControl(new IKnobMultiControl(this, 363, 178, mFilterEnvelopeAmount, &smallKnobBitmap));

  AttachGraphics(pGraphics);
  CreatePresets();

  mMIDIReceiver.noteOn.Connect(this, &Synthesis::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &Synthesis::onNoteOff);
  mEnvelopeGenerator.beganEnvelopeCycle.Connect(this, &Synthesis::onBeganEnvelopeCycle);
  mEnvelopeGenerator.finishedEnvelopeCycle.Connect(this, &Synthesis::onFinishedEnvelopeCycle);

  mLFO.setMode(OSCILLATOR_MODE_TRIANGLE);
  mLFO.setFrequency(6.0);
  mLFO.setMuted(false);
}

Synthesis::~Synthesis() {}

void Synthesis::ProcessDoubleReplacing(
	double** inputs, 
	double** outputs, 
	int nFrames) 
{
	// Mutex is already locked for us

	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];

	processVirtualKeyboard();

	for (int i = 0; i < nFrames; ++i) {
		mMIDIReceiver.advance();
		int velocity = mMIDIReceiver.getLastVelocity();
		double lfoFilterModulation = mLFO.nextSample() * lfoFilterModAmount;
		mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
		mFilter.setCutoffMod((mFilterEnvelopeGenerator.nextSample() * filterEnvelopeAmount) + lfoFilterModulation);
		leftOutput[i] = rightOutput[i] = mFilter.process(mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0);
	}

	mMIDIReceiver.Flush(nFrames);
}

void Synthesis::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mEnvelopeGenerator.setSampleRate(GetSampleRate());
  mFilterEnvelopeGenerator.setSampleRate(GetSampleRate());
  mLFO.setSampleRate(GetSampleRate());
}

void Synthesis::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);
	switch (paramIdx) {
		case mWaveform:
			mOscillator.setMode(static_cast<OscillatorMode>(GetParam(mWaveform)->Int()));
			break;
		case mAttack:
		case mDecay:
		case mSustain:
		case mRelease:
			mEnvelopeGenerator.setStageValue(static_cast<EnvelopeGenerator::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
			break;
		case mFilterCutoff:
			mFilter.setCutoff(GetParam(paramIdx)->Value());
			break;
		case mFilterResonance:
			mFilter.setResonance(GetParam(paramIdx)->Value());
			break;
		case mFilterMode:
			mFilter.setFilterMode(static_cast<Filter::FilterMode>(GetParam(paramIdx)->Int()));
			break;
		case mFilterAttack:
			mFilterEnvelopeGenerator.setStageValue(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK, GetParam(paramIdx)->Value());
			break;
		case mFilterDecay:
			mFilterEnvelopeGenerator.setStageValue(EnvelopeGenerator::ENVELOPE_STAGE_DECAY, GetParam(paramIdx)->Value());
			break;
		case mFilterSustain:
			mFilterEnvelopeGenerator.setStageValue(EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN, GetParam(paramIdx)->Value());
			break;
		case mFilterRelease:
			mFilterEnvelopeGenerator.setStageValue(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE, GetParam(paramIdx)->Value());
			break;
		case mFilterEnvelopeAmount:
			filterEnvelopeAmount = GetParam(paramIdx)->Value();
			break;
	}
}

void Synthesis::CreatePresets() {
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
	mVirtualKeyboard->SetDirty();
}

void Synthesis::processVirtualKeyboard() {
	IKeyboardControl* virtualKeyboard = (IKeyboardControl*) mVirtualKeyboard;
	int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;
	if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber &&
		virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
		// Note number has changed from valid to invalid
		IMidiMsg midiMessage;
		midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
		mMIDIReceiver.onMessageReceived(&midiMessage);
	}
	if (virtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber &&
		virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
		// Valid new key pressed
		IMidiMsg midiMessage;
		midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
		mMIDIReceiver.onMessageReceived(&midiMessage);
	}

	lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;
}