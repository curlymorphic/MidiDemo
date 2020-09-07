#include "plugin.hpp"

// an example of a cv - midi module for VCV rack
// with predefined cc outputs
//based on Cv-CC.cpp


namespace MidiDemo {

	// define the number of controllers once
	// to ensure code only needs to be updated in a
	// single location if the value changes

	static constexpr int numControllers = 35;

	// array of cc to use
	// I have renames this as we are not learning
	// so ccs is a more suitable name

	int ccs[numControllers] = {20, 21, 23, 24, 25, 26, 28, 29, 46, 47,
							   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
							   59, 60, 61, 62, 63, 80, 81, 82, 83, 84, 85,
							   86, 87, 88};



	// MidiOutput has been copied from VC_CC.cpp
	// all midi output uses this object
	// the interface must be selected in the ModuleWidget
	// to send a CCC use setValue()
	// it will not sent the same cc if the value is unchanged


	struct CCMidiOutput : midi::Output {
		int lastValues[128];

		CCMidiOutput() {
			reset();
		}

		void reset() {
			for (int n = 0; n < 128; n++) {
				lastValues[n] = -1;
			}
		}

		void setValue(int value, int cc) {
			if (value == lastValues[cc])
				return;
			lastValues[cc] = value;
			// CC
			midi::Message m;
			m.setStatus(0xb);
			m.setNote(cc);
			m.setValue(value);
			sendMessage(m);
		}
	};


	struct CvOut : Module {
		enum ParamIds {
			NUM_PARAMS
		};
		enum InputIds {
			ENUMS(CC_INPUTS, numControllers),
			NUM_INPUTS
		};
		enum OutputIds {
			NUM_OUTPUTS
		};
		enum LightIds {
			NUM_LIGHTS
		};

		// midi output member
		CCMidiOutput midiOutput;

		// the midi is not sent at audio rate
		// this counter is used to only check the
		// CV inputs and send midi at set durations
		// in process()
		float rateLimitPhase;

		CvOut() {
			config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
			onReset();
		}

		// when the module is reset
		// reset the midi contollers within vcv
		void onReset() override {
			midiOutput.reset();
			midiOutput.midi::Output::reset();
		}

		// copied from CV_CC.cpp
		void process(const ProcessArgs &args) override {

			// Timer code, so the midi is not sent on every audio frame
			// the interval is set by rateLimiterPeriod
			// currently set to 0.010 seconds = 10ms
			const float rateLimiterPeriod = 0.010f;
			rateLimitPhase += args.sampleTime / rateLimiterPeriod;
			if (rateLimitPhase >= 1.f) {
				rateLimitPhase -= 1.f;
			} else {
				return;
			}



			// loop over all the inputs and send cv
			// not I have changed the condition in the for loop
			// to use numControllers defined at the top of this file

			for (int i = 0; i < numControllers; i++) {
				int value = (int) std::round(inputs[CC_INPUTS + i].getVoltage() / 10.f * 127);
				value = clamp(value, 0, 127);
				midiOutput.setValue(value, ccs[i]);
			}
		}


		// saves the selected midi interface
		json_t *dataToJson() override {
			json_t *rootJ = json_object();
			json_object_set_new(rootJ, "midi", midiOutput.toJson());
			return rootJ;
		}

		// restore the samed midi interface
		void dataFromJson(json_t* rootJ) override {
			json_t* midiJ = json_object_get(rootJ, "midi");
			if (midiJ)
				midiOutput.fromJson(midiJ);
		}

	};

	struct CvOutWidget : ModuleWidget {
		CvOutWidget(CvOut *module) {
			setModule(module);
			setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CvOut.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(
					Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			// place the cv inputs on the ui
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 20)), module, CvOut::CC_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 20)), module, CvOut::CC_INPUTS + 1));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 20)), module, CvOut::CC_INPUTS + 2));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(40, 20)), module, CvOut::CC_INPUTS + 3));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 20)), module, CvOut::CC_INPUTS + 4));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 20)), module, CvOut::CC_INPUTS + 5));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 20)), module, CvOut::CC_INPUTS + 6));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(80, 20)), module, CvOut::CC_INPUTS + 7));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 30)), module, CvOut::CC_INPUTS + 8));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 30)), module, CvOut::CC_INPUTS + 9));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 30)), module, CvOut::CC_INPUTS + 10));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(40, 30)), module, CvOut::CC_INPUTS + 11));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 30)), module, CvOut::CC_INPUTS + 12));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 30)), module, CvOut::CC_INPUTS + 13));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 30)), module, CvOut::CC_INPUTS + 14));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(80, 30)), module, CvOut::CC_INPUTS + 15));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 40)), module, CvOut::CC_INPUTS + 16));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 40)), module, CvOut::CC_INPUTS + 17));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 40)), module, CvOut::CC_INPUTS + 18));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(40, 40)), module, CvOut::CC_INPUTS + 19));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 40)), module, CvOut::CC_INPUTS + 20));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 40)), module, CvOut::CC_INPUTS + 21));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 40)), module, CvOut::CC_INPUTS + 22));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(80, 40)), module, CvOut::CC_INPUTS + 23));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50)), module, CvOut::CC_INPUTS + 24));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 50)), module, CvOut::CC_INPUTS + 25));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 50)), module, CvOut::CC_INPUTS + 26));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(40, 50)), module, CvOut::CC_INPUTS + 27));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 50)), module, CvOut::CC_INPUTS + 28));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 50)), module, CvOut::CC_INPUTS + 29));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 50)), module, CvOut::CC_INPUTS + 30));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(80, 50)), module, CvOut::CC_INPUTS + 31));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 60)), module, CvOut::CC_INPUTS + 32));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 60)), module, CvOut::CC_INPUTS + 33));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 60)), module, CvOut::CC_INPUTS + 34));


			// add the midi selector widget to the ui
			auto* midiAInWidget = createWidget<MidiWidget> (mm2px (Vec(30, 80)));
			midiAInWidget->box.size = mm2px (Vec (40, 30));
			midiAInWidget->setMidiPort (module ? &module->midiOutput : NULL);
			addChild (midiAInWidget);
		}
	};
}


Model *modelCvOut = createModel<MidiDemo::CvOut, MidiDemo::CvOutWidget>("CvOut");