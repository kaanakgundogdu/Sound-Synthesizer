#include <iostream>
#include "olcNoiseMaker.h"


struct EnvelopeASDR
{
	double attack_time;
	double decay_time;
	double release_time;

	double start_amplitude;
	double sustain_amplitude;

	double trigger_on_time;
	double trigger_off_time;

	bool is_note_on;

	EnvelopeASDR() {
		attack_time = 0.10;
		decay_time = 0.01;
		release_time = 0.20;

		start_amplitude = 1.0;
		sustain_amplitude = 0.8;

		trigger_on_time = 0.0;
		trigger_off_time = 0.0;

		is_note_on = false;
	}

	double get_amplitude(double d_time) {
		double amplitude = 0.0;
		double life_time = d_time - trigger_on_time;

		if (is_note_on) {

			//Attack
			if (life_time <= attack_time)
			{
				amplitude = (life_time / attack_time) * start_amplitude;
			}
			//decay
			if (life_time > attack_time && life_time <= (attack_time + decay_time)) {
				amplitude = ((life_time - attack_time) / decay_time) *
					(sustain_amplitude - start_amplitude) + start_amplitude;
			}
			//sustain
			if (life_time > (attack_time + decay_time)) {
				amplitude = sustain_amplitude;
			}

		}

		else {
			amplitude = ((d_time - trigger_off_time) / release_time) * (0.0 - sustain_amplitude) + sustain_amplitude;
		}

		if (amplitude < 0.0001)
			amplitude = 0;

		return amplitude;
	}

	void note_on(double time_on) {
		trigger_on_time = time_on;
		is_note_on = true;
	}

	void note_off(double time_off) {
		trigger_off_time = time_off;
		is_note_on = false;
	}

};



std::atomic<double> d_frequencyOutput = 0.0;
double d_octaveBaseFrequency = 110.0;
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);
EnvelopeASDR envelope;

void print_keyboard(const std::vector<std::wstring>& devices);


double w(double d_hertz) {
	return d_hertz * 2.0 * PI;
}

double oscillator(double d_hertz, double d_time, int sound_type=0, double lfo_hertz=0.0, double lfo_amplitude=0.0) {

	double freq = w(d_hertz) * d_time + lfo_amplitude * d_hertz * sin(w (lfo_hertz) * d_time);

	switch (sound_type)
	{
	case 0://sin
		return sin(freq);
	case 1://square
		return sin(freq) > 0.0 ? 1.0 : -1.0;
	case 2://triang
		return asin(sin(freq)) * (2.0 / PI);

	case 3: {//saw ana
		double output_ = 0.0;
		for (double i = 1.0; i < 100.0; i++)
		{
			output_ += (sin(i * freq)) / i;
		}
		return output_ * (2.0 / PI);
	}

	case 4: //saw dig
		return (2.0 / PI) * (d_hertz * PI * fmod(d_time, 1.0 / d_hertz) - (PI / 2.0));

	case 5: //random noise
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;

	default:
		return 0.0;
	}
}




double MakeNoise(double time_)
{

	double output_ = envelope.get_amplitude(time_) *
		(
			+ 1.0 * oscillator(d_frequencyOutput, time_, 2 , 5.0,0.01)
			+ 0.5 * oscillator(d_frequencyOutput * 1.5 , time_, 2 )
			+ 0.25 * oscillator(d_frequencyOutput * 2.0 , time_, 2 )
			+ 0.05 * oscillator(0, time_, 5)
		);


	return output_ * 0.4;
}

int main()
{
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
	
	print_keyboard(devices);

	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);


	sound.SetUserFunction(MakeNoise);


	int current_key = -1;
	bool is_key_pressed = false;

	while (1)
	{
		is_key_pressed = false;
		for (int k = 0; k < 16; k++)
		{
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000)
			{
				if (current_key != k)
				{
					d_frequencyOutput = d_octaveBaseFrequency * pow(d12thRootOf2, k);

					envelope.note_on(sound.GetTime());

					std::wcout << "\rNote On : " << sound.GetTime() << "s " << d_frequencyOutput << "Hz";
					current_key = k;
				}

				is_key_pressed = true;
			}
		}

		if (!is_key_pressed)
		{
			if (current_key != -1)
			{
				envelope.note_off(sound.GetTime());

				std::wcout << "\rNote Off: " << sound.GetTime() << "s                        ";
				current_key = -1;
			}
		}
	}

	return 0;
}


void print_keyboard(const std::vector<std::wstring>& devices) {
	
	for (auto d : devices) 
		std::wcout << "Found Output Device: " << d << "\n";
	
	std::wcout << "Using Device: " << devices[0] << "\n";

	std::wcout << "\n" <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << "\n" <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << "\n" <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << "\n" <<
		"|     |     |     |     |     |     |     |     |     |     |" << "\n" <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << "\n" <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << "\n" << "\n";
}
