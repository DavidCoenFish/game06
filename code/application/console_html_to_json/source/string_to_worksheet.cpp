#include "string_to_worksheet.h"
#include "worksheet.h"

#include <pugixml.hpp>

namespace
{
#if 0
    const char* GetAttribute(const pugi::xml_node& in_node, const char* in_key)
    {
        return in_node.attribute(in_key).as_string("");
    }
    const char* GetParentAttribute(const pugi::xml_node& in_node, const char* in_key)
    {
        return in_node.parent().attribute(in_key).as_string("");
    }
#endif

    const char* GetChildText(const pugi::xml_node& in_node)
    {
        return in_node.first_child().text().get();
    }

    typedef std::map<std::string, std::shared_ptr<Worksheet>> TWorksheetMap;
    class SimpleWalker: public pugi::xml_tree_walker
    {
    public:
        SimpleWalker() = delete;
        SimpleWalker& operator=(const SimpleWalker&) = delete;
        SimpleWalker(const SimpleWalker&) = delete;

        SimpleWalker(TWorksheetMap& out_worksheet_map, const std::string& in_worksheet_name)
            : _worksheet_name(in_worksheet_name)
            , _worksheet_map(out_worksheet_map)
        {
            _current_worksheet = std::make_shared<Worksheet>();
            _worksheet_map[_worksheet_name] = _current_worksheet;

            printf("    parse worksheet:%s\n", _worksheet_name.c_str());
        }

        ~SimpleWalker()
        {
            // append any remaining data to the back of the last worksheet
            StartNewRow();
        }

    private:
        void StartNewRow()
        {
            if ((nullptr != _current_worksheet) && (true == _current_row_has_valid_data))
            {
                _current_worksheet->AppendRow(_current_row);
            }

            _current_row.clear();
            _current_row_has_valid_data = false;
        }

        virtual bool for_each(pugi::xml_node& in_node)
        {
            //const char* parent_id = GetParentAttribute(in_node, "id");
            //const char* id = GetAttribute(in_node, "id");

            // for each row in the table
            if (0 == strcmp(in_node.name(), "tr"))
            {
                StartNewRow();
            }

            // ignore "th", the row index is displayed, but that is not relevant data for us, so ignored

            // the table data row. to preserve order, need to add empty cells, but if all the cells in a row are empty, ignore the row, bool _current_row_has_valid_data
            if (0 == strcmp(in_node.name(), "td"))
            {
                const std::string value = GetChildText(in_node);
                if (nullptr != _current_worksheet)
                {
                    _current_row_has_valid_data |= (false == value.empty());
                    _current_row.push_back(value);
                }
            }

            return true; // continue traversal
        }

    private:
        const std::string _worksheet_name;
        TWorksheetMap& _worksheet_map;

        std::shared_ptr<Worksheet> _current_worksheet;
        std::vector<std::string> _current_row;
        bool _current_row_has_valid_data;
    };

}

const bool StringToWorksheet::DealSourceData(std::map<std::string, std::shared_ptr<Worksheet>>& out_source_data, const std::string& in_source_data, const std::string& in_worksheet_name)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(in_source_data.c_str());

    if (pugi::status_ok != result.status)
    {
        return false;
    }

    {
        SimpleWalker simple_walker(out_source_data, in_worksheet_name);
        doc.traverse(simple_walker);
    }

    return true;
}
