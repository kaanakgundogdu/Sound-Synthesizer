#include <iostream>
#include "olcNoiseMaker.h"

double w(double d_hertz) {
	return d_hertz * 2.0 * PI;
}

double oscillator(double d_hertz, double d_time, int sound_type) {

	switch (sound_type)
	{
	case 0:
		return sin(w(d_hertz) * d_time);
	case 1:
		return sin(w(d_hertz) * d_time) > 0.0 ? 1.0 : -1.0;
	case 2:
		return asin(sin(w(d_hertz) * d_time)) * (2.0 / PI);

	case 3: {
		double xd = 0.0;
		for (double i = 1.0; i < 100.0; i++)
		{
			xd += (sin(i * w(d_hertz) * d_time)) / i;
		}
		return xd * (2.0 / PI);
	}

	
	case 4:
		return (2.0 / PI) * (d_hertz * PI * fmod(d_time, 1.0 / d_hertz) - (PI / 2.0));

	default:
		return 0.0;
	}
}

std::atomic<double> d_frequencyOutput = 0.0;
double d_octaveBaseFrequency = 110.0; 	
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		


void print_keyboard(const std::vector<std::wstring>& devices);


double MakeNoise(double d_time)
{
	double dOutput = oscillator(d_frequencyOutput, d_time, 4);
	return dOutput * 0.4; 
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
				std::wcout << "\rNote Off: " << sound.GetTime() << "s                        ";
				current_key = -1;
			}

			d_frequencyOutput = 0.0;
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
