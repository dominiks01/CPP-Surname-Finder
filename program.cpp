#include <cctype>
#include <cinttypes>
#include <cstddef>
#include <cstring>
#include <map>
#include <string>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <codecvt>
#include <locale>
#include <iomanip>
#include <sstream>
#include <bits/stdc++.h>

size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* output){
    size_t totalSize = size * nmemb;
    output->append(contents, totalSize);
    return totalSize;
}

const std::string red("\033[0;31m");
const std::string green("\033[1;32m");
const std::string yellow("\033[1;33m");
const std::string cyan("\033[0;36m");
const std::string magenta("\033[0;35m");
const std::string reset("\033[0m");

std::vector<std::pair<std::string, std::string>> markers = {
    std::make_pair(std::string("<p class=\"is-size-4 has-text-black-ter is-size-5-touch mb-3\">"), std::string("</p>")),
    std::make_pair(std::string("<p class=\"is-size-4 has-text-black-ter is-size-5-touch mb-3\">"), std::string("</p>")),
    std::make_pair(std::string("<p class=\"is-size-2 has-text-weight-bold has-text-centered-mobile is-size-3-touch\">"), std::string("</p>")),
    std::make_pair(std::string("<p class=\"is-size-2 has-text-weight-bold has-text-centered-mobile is-size-3-touch\">"), std::string("</p>")),
    std::make_pair(std::string("<p class=\"is-size-2 is-size-3-touch has-text-weight-bold has-text-centered has-text-light is-size-3-touch\">"), std::string("</p>")),
    std::make_pair(std::string("<p class=\"is-size-5 is-size-6-touch pl-5\">"), std::string("</p>"))
};

std::vector<std::string> result_label = {
    "General Info",
    "Number of people overall",
    "Number of females", 
    "Number of males",
    "Surname popularity"
};

std::vector<std::string> occurance_in_poland; 
std::map<std::string, std::string>result;

void ExtractData(const std::string& content, std::vector<std::pair<std::string, std::string>> markers, std::map<std::string, std::string>& result){
    std::string response; 
    size_t starting_position = 0, end_position = 0;

    for(std::vector<std::pair<std::string, std::string>> ::iterator it = markers.begin(); it != markers.end() - 1; it++){
        starting_position = content.find(it->first, end_position);

        if (starting_position == std::string::npos)
            continue;
        
        end_position = content.find(it->second, starting_position + it->first.length());

        if (end_position == std::string::npos)
            continue;   

        std::string response = content.substr(starting_position + it->first.length(), end_position - starting_position - it->first.length());
        std::string final;
        int counter = 0;

        for(std::string::const_iterator final_iterator = response.begin(); final_iterator!=response.end(); final_iterator++){
            if (*final_iterator == '<' || *final_iterator == '>')
                counter++;
            else if (counter % 2 == 0)
                final += *final_iterator;
        }

        result.insert({result_label[std::distance(markers.begin(), it)], final});
    }

    for(int i = 0; i < 16; i++){
        starting_position = content.find(markers.back().first, end_position);

        if (starting_position == std::string::npos)
            continue;

        end_position = content.find(markers.back().second, starting_position + markers.back().first.length());

         if (end_position == std::string::npos)
            continue;

        std::string response = content.substr(starting_position + markers.back().first.length() + 1, end_position - starting_position);
        
        int counter = 0;        
        std::string final;

        for(std::string::const_iterator final_iterator = response.begin(); final_iterator!=response.end(); final_iterator++){
            if (*final_iterator == '<' || *final_iterator == '>')
                counter++;
            else if (counter % 2 == 0)
                final += *final_iterator;
        }

        if (final.size() >= 100)
            return; 

        occurance_in_poland.push_back(final);
    }
}

int main(){
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    std::string name;

    std::cout << "Wpisz nazwisko do sprawdzenia: ";
    std::cin >> name;

    char url[] = u8"https://polskienazwiska.pl/n/";

    std::string valid_url = "";

    for(std::string::size_type i = 0; i < name.size(); i++){

        if(std::isalpha(name[i]))
            valid_url += name[i];
        else {
                std::stringstream ss;
                ss << std::hex << std::setw(2) << static_cast<int>(name[i]);
                std::string encoded = "%" + ss.str();
                transform(encoded.begin(), encoded.end(), encoded.begin(), ::toupper);

                valid_url += ("%" + encoded.substr(encoded.size() - 2));
        }
    }

    valid_url = url + valid_url;

    if (curl){
        
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, valid_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK){
            std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
        } else {
            ExtractData(response, markers, result);

            if(result.count("General Info") > 0)
                std::cout << green << "\n" <<result["General Info"].substr(1) << "\n\n";
            else 
                std::cout << red <<"Nie znaleziono podanego nazwiska\n";

            if(result.count("Number of people overall") > 0)
                std::cout << cyan << result["Number of people overall"].substr(1) << "\n";

            if(result.count("Number of males") > 0)
                std::cout << cyan << "Liczba mężczyzn: "<< std::left << red << result["Number of males"] << "\n";

            if(result.count("Number of females") > 0)
                std::cout << cyan << "Liczba kobiet: "  << std::left << red << result["Number of females"] << "\n";

            if(result.count("Surname popularity") > 0)
                std::cout << cyan << std::left << "Popularność nazwiska w Polsce: " << red <<result["Surname popularity"] << "\n\n";

            if(occurance_in_poland.size() != 0){
                std::cout << cyan << "Występowanie w Polsce: \n";

                for(std::string voivodeship : occurance_in_poland)
                    std::cout << cyan << voivodeship << "\n";

                std::cout << "\n";
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}