// TEA encryption.cpp : Defines the entry point for the console application.
//
#include "std_lib_facilities.h"
#include <windows.h>
void encipher(
	const unsigned long *const v,
	unsigned long *const w,
	const unsigned long * const k, int iterations = 32)
{
	static_assert(sizeof(long) == 4, "size ò long wrong for TEA");
	unsigned long y = v[0];
	unsigned long z = v[1];
	unsigned long sum = 0;
	const unsigned long delta = 0x9E3779B9;
	for (unsigned long n = iterations; n-- > 0; ) {
		y += (z << 4 ^ z >> 5) + z^sum + k[sum & 3];
		sum += delta;
		z += (y << 4 ^ y >> 5) + y^sum + k[sum >> 11 & 3];
	}
	w[0] = y;
	w[1] = z;
}
void decipher(
	const unsigned long *const v,
	unsigned long *const w,
	const unsigned long * const k, int iterations = 32)
{
	static_assert(sizeof(long) == 4, "size of long wrong for TEA");
	unsigned long y = v[0];
	unsigned long z = v[1];
	unsigned long sum = 0xC6EF3720;
	const unsigned long delta = 0x9E3779B9;
	// sum = delta<<5, in general sum = delta * n
	for (unsigned long n = iterations; n-- > 0; ) {
		z -= (y << 4 ^ y >> 5) + y ^ sum + k[sum >> 11 & 3];
		sum -= delta;
		y -= (z << 4 ^ z >> 5) + z ^ sum + k[sum & 3];
	}
	w[0] = y;
	w[1] = z;
}

int main() // sender
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	const int nchar = 2 * sizeof(long);
	const int kchar = 2 * nchar;
	string op;
	string key;
	string infile;
	string outfile;
	SetConsoleTextAttribute(hConsole, 11);
	cout << "Input mode (encrypt/decrypt/optional_encrypt/optional_decrypt): ";
	SetConsoleTextAttribute(hConsole, 12);
	string mode;
	cin >> mode;
	if (mode == "optional_decrypt") {
		SetConsoleTextAttribute(hConsole, 11);
		cout << "Please enter input file name, output file name, and key\n(and number of iterations, which can be skipped by Enter:\n";
		SetConsoleTextAttribute(hConsole, 12);
		int iterations;
		cin >> infile >> outfile >> key >> iterations;
		while (key.size() < kchar) key += '0'; // pad key
		ifstream inf(infile);
		ofstream outf(outfile);
		if (!inf || !outf) error("bad file name");
		const unsigned long* k =
			reinterpret_cast<const unsigned long*>(key.data());
		unsigned long inptr[2];
		char outbuf[nchar + 1];
		outbuf[nchar] = 0; // terminator
		unsigned long* outptr = reinterpret_cast<unsigned long*>(outbuf);
		inf.setf(ios_base::hex, ios_base::basefield); // use hexadecimal input
		while (inf >> inptr[0] >> inptr[1]) {
			decipher(inptr, outptr, k, iterations);
			outf << outbuf;
		}
		inf.close();
		outf.close();
	}
	else if (mode == "optional_encrypt") {
		SetConsoleTextAttribute(hConsole, 11);
		cout << "Please enter input file name, output file name, and key\n(and number of iterations, which can be skipped by Enter:\n";
		SetConsoleTextAttribute(hConsole, 12);
		int iterations;
		cin >> infile >> outfile >> key >> iterations;
		while (key.size() < kchar) key += '0'; // pad key
		ifstream inf(infile);
		ofstream outf(outfile);
		if (!inf || !outf) error("bad file name");
		const unsigned long* k =
			reinterpret_cast<const unsigned long*>(key.data());
		unsigned long outptr[2];
		char inbuf[nchar];
		unsigned long* inptr = reinterpret_cast<unsigned long*>(inbuf);
		int count = 0;
		while (inf.get(inbuf[count])) {
			outf << hex; // use hexadecimal output
			if (++count == nchar) {
				encipher(inptr, outptr, k, iterations);
				// pad with leading zeros:
				outf << setw(8) << setfill('0') << outptr[0] << ' '
					<< setw(8) << setfill('0') << outptr[1] << ' ';
				count = 0;
			}
		}
		if (count) { // pad
			while (count != nchar) inbuf[count++] = '0';
			encipher(inptr, outptr, k, iterations);
			outf << outptr[0] << ' ' << outptr[1] << ' ';
		}
		inf.close();
		outf.close();
	}
	else if (mode == "encrypt") {
		SetConsoleTextAttribute(hConsole, 11);
		cout << "Please enter input file name, output file name, and key:\n";
		SetConsoleTextAttribute(hConsole, 12);
		cin >> infile >> outfile >> key;
		while (key.size() < kchar) key += '0'; // pad key
		ifstream inf(infile);
		ofstream outf(outfile);
		if (!inf || !outf) error("bad file name");
		const unsigned long* k =
			reinterpret_cast<const unsigned long*>(key.data());
		unsigned long outptr[2];
		char inbuf[nchar];
		unsigned long* inptr = reinterpret_cast<unsigned long*>(inbuf);
		int count = 0;
		while (inf.get(inbuf[count])) {
			outf << hex; // use hexadecimal output
			if (++count == nchar) {
				encipher(inptr, outptr, k);
				// pad with leading zeros:
				outf << setw(8) << setfill('0') << outptr[0] << ' '
					<< setw(8) << setfill('0') << outptr[1] << ' ';
				count = 0;
			}
		}
		if (count) { // pad
			while (count != nchar) inbuf[count++] = '0';
			encipher(inptr, outptr, k);
			outf << outptr[0] << ' ' << outptr[1] << ' ';
		}
		inf.close();
		outf.close();
	}
	else if (mode == "decrypt") {
		SetConsoleTextAttribute(hConsole, 11);
		cout << "Please enter input file name, output file name, and key:\n";
		SetConsoleTextAttribute(hConsole, 12);
		cin >> infile >> outfile >> key;
		while (key.size() < kchar) key += '0'; // pad key
		ifstream inf(infile);
		ofstream outf(outfile);
		if (!inf || !outf) error("bad file name");
		const unsigned long* k =
			reinterpret_cast<const unsigned long*>(key.data());
		unsigned long inptr[2];
		char outbuf[nchar + 1];
		outbuf[nchar] = 0; // terminator
		unsigned long* outptr = reinterpret_cast<unsigned long*>(outbuf);
		inf.setf(ios_base::hex, ios_base::basefield); // use hexadecimal input
		while (inf >> inptr[0] >> inptr[1]) {
			decipher(inptr, outptr, k);
			outf << outbuf;
		}
		inf.close();
		outf.close();
	}
	return 0;
}
