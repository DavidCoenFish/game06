#include "main.h"
#include "state.h"
#include "string_to_worksheet.h"
#include "worksheet_to_json.h"
#include "cursor.h"

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <locale>
#include <codecvt>
#include <string>

/*
google sheets webpage menu bar File>Download>Web Page (.html) -> downloads a zip of each sheet as a html document

https://docs.google.com/spreadsheets/d/1SaIO66R9qPoMUqq8C-PDZ-vMbLdrqfQH7dlKiuJQYNg/edit?gid=199073173#gid=199073173
-i "data\sample_html_to_json" -o "output\locale.json" --sheet5th toc -d
-i "data\sample_html_to_json" -o "G:\development\game06\data\lqrpg\lqrpg\data.json" --sheet5th toc -d


*/

namespace
{
    enum ApplicationExitCode : int
    {
        TNone = 0,
        TBadParam = -1,
        TOutputFileCreateFailed = -2,
        TOutputFileWriteFailed = -4,
        TInputFileFail = -8,
        TInputURLFail = -16,
        TUnimplemented = -32,
        TParseSource = -64,
        TWorksheetError = -128
    };

    void Help()
    {
        printf(R"(
usage:
	drive-json [options]
		
sample:
	html_to_json.exe --input ./input/character_only.html --sheet5th toc --outjson ./output/character.json
		add the data from the saved webpage at the given path in 3rd normalised form, and save result data as json file at output
		
options:
-h --help -?
	generate this text
-i --input <input filepath>
-d --doctorInput // default html saved out from google html is not well formed xml, trim the file down to what is inside <body...>...</body> tag
-s --dataSet <lable>
	add lable to the allowed dataSet for filtering sheet3rdKeyValue sheets
-3 --sheet3rd <worksheet name>
	append the result data with data sourced from google spread sheet worksheet in 3rd normalised form and trated as id : object
-5 --sheet5th <worksheet name>
	append the result data with data sourced from google spread sheet worksheet in 5th normalised form and treated as key : value
-v --sheet3rdKeyValue <worksheet name>
	append the result data with data sourced from google spread sheet worksheet in 3rd normalised form filtered by dataset and treated as key : value
-o --outjson <output json filepath>
	specify the file location to save a json  representation of the result data
)");
    }

    const std::string DoctorInput(const std::string& in_source_data)
    {
        const std::string skip_past_token = "<link type=\"text/css\" rel=\"stylesheet\" href=\"resources/sheet.css\" >";
        const auto found = in_source_data.find(skip_past_token);
        if (std::string::npos != found)
        {
            return in_source_data.substr(found + skip_past_token.length());
        }
        return in_source_data;
    }

    // default html saved out from google html is not well formed xml, trim the file down to what is inside <body...>...</body> tag
    const int DealInputFile(std::map<std::string, std::shared_ptr<Worksheet>>& out_source_data, const std::string& in_input_file_path, const bool in_doctor_input)
    {
        printf("Input file:%s\n", in_input_file_path.c_str());

        int result = ApplicationExitCode::TNone;

        for (const auto& entry : std::filesystem::directory_iterator(in_input_file_path))
        {

            //get the source string
            //https://stackoverflow.com/questions/20902945/reading-a-string-from-file-c
            std::string source_data;
            {
                const std::ifstream input_stream(entry, std::ios_base::binary);

                if (input_stream.fail()) 
                {
                    result |= ApplicationExitCode::TInputFileFail;
                    continue;
                }

                std::stringstream buffer;
                buffer << input_stream.rdbuf();

                source_data = buffer.str();
            }

            if (true == in_doctor_input)
            {
                source_data = DoctorInput(source_data);
            }

            //convert source string to Worksheets
            const auto sheet_name = std::filesystem::path(entry).stem().string();
            if (false == StringToWorksheet::DealSourceData(out_source_data, source_data, sheet_name))
            {
                result |= ApplicationExitCode::TParseSource;
                continue;
            }

        }


        return result;
    }

    const int SaveStringToFile(const std::string& in_data, const std::filesystem::path& in_file_path)
    {
        //printf("Save file:%s\n", in_file_path.c_str());

        std::filesystem::path dir_path = std::filesystem::path(in_file_path).parent_path();
        std::filesystem::create_directories(dir_path);

        std::ofstream outFile(in_file_path, std::ios::out | std::ios::binary | std::ios::ate);
        if (!outFile)
        {
            printf("output file creation failed\n");
            return ApplicationExitCode::TOutputFileCreateFailed;
        }

        outFile.write(in_data.c_str(), in_data.size());
        if (!outFile)
        {
            printf("output file write failed\n");
            return ApplicationExitCode::TOutputFileWriteFailed;
        }

        outFile.close();

        return ApplicationExitCode::TNone;

    }

    const int DealOutput(const nlohmann::json& in_output_data, const std::string& in_output_file_path)
    {
        printf("DealOutput:%s\n", in_output_file_path.c_str());

        const std::string data = in_output_data.dump(1, '\t');

        return SaveStringToFile(data.c_str(), in_output_file_path);
    }

}


int main(const int in_argc, const char* const in_argv[]) 
{
    int result = ApplicationExitCode::TNone;

    State state;

    // consume cmd line
    if (in_argc <= 1)
    {
        state.do_help = true;
    }
    else
    {
        for (int index = 1; index < in_argc; ++index)
        {
            const std::string token = in_argv[index];
            if ((token == "-h") || (token == "--help") || (token == "-?"))
            {
                state.do_help = true;
            }
            else if (((token == "-i") || (token == "--input")) && (index + 1 < in_argc))
            {
                index += 1;
                state.input_file_path = in_argv[index];
            }
            else if ((token == "-d") || (token == "--doctorInput"))
            {
                state.doctor_input = true;
            }
            else if (((token == "-s") || (token == "--dataSet")) && (index + 1 < in_argc))
            {
                index += 1;
                state.data_set.insert(in_argv[index]);
            }
            else if (((token == "-3") || (token == "--sheet3rd")) && (index + 1 < in_argc))
            {
                index += 1;
                state.sheet_3rd_worksheet_name = in_argv[index];
            }
            else if (((token == "-5") || (token == "--sheet5th")) && (index + 1 < in_argc))
            {
                index += 1;
                state.sheet_5th_worksheet_name = in_argv[index];
            }
            else if (((token == "-v") || (token == "--sheet3rdKeyValue")) && (index + 1 < in_argc))
            {
                index += 1;
                state.sheet_3rd_key_value_worksheet_name = in_argv[index];
            }
            else if (((token == "-o") || (token == "--outjson")) && (index + 1 < in_argc))
            {
                index += 1;
                state.output_file_path = in_argv[index];
            }
            else
            {
                printf("unknown cmd line param:%s", token.c_str());
                state.do_help = true;
                result |= ApplicationExitCode::TBadParam;
            }
        }
    }

    // perform actions

    if (true == state.do_help)
    {
        Help();
    }

    // input
    if (false == state.input_file_path.empty())
    {
        result |= DealInputFile(state.source_data, state.input_file_path, state.doctor_input);
    }

    // process
    if (false == state.sheet_3rd_worksheet_name.empty())
    {
        if (false == WorksheetToJson::Deal3rd(
            state.output_data,
            state.source_data,
            state.sheet_3rd_worksheet_name,
            Cursor(state.data_set, false)
            ))
        {
            result |= TWorksheetError;
        }
    }
    if (false == state.sheet_5th_worksheet_name.empty())
    {
        if (false == WorksheetToJson::Deal5th(
            state.output_data,
            state.source_data,
            state.sheet_5th_worksheet_name,
            Cursor(state.data_set, false)
            ))
        {
            result |= TWorksheetError;
        }
    }
    if (false == state.sheet_3rd_key_value_worksheet_name.empty())
    {
        if (false == WorksheetToJson::Deal3rdKeyValue(
            state.output_data,
            state.source_data,
            state.sheet_3rd_key_value_worksheet_name,
            Cursor(state.data_set, true)
            ))
        {
            result |= TWorksheetError;
        }
    }

    // output
    if (false == state.output_file_path.empty())
    {
        result |= DealOutput(state.output_data, state.output_file_path);
    }

    return result;
}

