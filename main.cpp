#include "sound_analyzer.h"

int main(int argc, char* argv[])
{
    std::string file_name;
    std::string file_name_expansion;
    int sec;
    char command = -1;

    if (argc == 2) {
        std::string arg1(argv[1]);
            file_name = argv[1];
            file_name_expansion = file_name.substr(file_name.size() - 4, 4);
            if (file_name_expansion != ".wav") {
                std::cout << "File should be .wav!";
                return 1;
            }
            //
            sAnalyser* analyser = new sAnalyser();
            if (analyser->initialize(argv[1]))
                analyser->decode();
            else return 1;
            //
            analyser->printInfo();
            std::cout << "Spectrum on which second you want to see? Write second from 1 to " << analyser->getDurationofAudio() << ": ";
            std::cin >> sec;
            while (!analyser->computeOneSecSpectrum(sec)) {
                std::cout << "Wrong second, try again: ";
                std::cin >> sec;
            }
            //
            while (command != 'e') {
                if (analyser->computeOneSecSpectrum(sec))
                    analyser->drawSpectrum(sec);
                std::cout << "\n\n You can move to another second, use left and right arrows. To exit press \"e\"\n\n";
                if (sec < analyser->getDurationofAudio()-1 && sec > 1)
                    std::cout << "        <- : Previous second\n        -> : Next second\n        e : Exit\n";
                else if (sec < analyser->getDurationofAudio() - 1)
                    std::cout << "        -> : Next second\n        e : Exit\n";
                else if(sec > 1)
                    std::cout << "        <- : Previous second\n        e : Exit\n";
       
                command = _getch();
                if (command == 75 && sec > 1) {
                    sec--;
                }
                else if (command == 77 && sec < analyser->getDurationofAudio() - 1) {
                    sec++;
                }
            }
        
            delete analyser;

    }
    else {
        std::cout << "Wrong parameters!" << std::endl;
        std::cout << "Your parameters: ";
        for (int i = 1; i < argc; i++) {
            std::cout << argv[i] << "; ";
        }
    }
    return 0;
}
