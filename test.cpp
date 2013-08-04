
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const char INPUT_FILENAME[] = "input.txt";
const char OUTPUT_FILENAME[] = "output.txt";

class Rule
{
public:
    Rule();
    ~Rule();
    Rule(const Rule& anotherRule);
    Rule& operator=(const Rule& anotherRule);

    void AddKey(const string& key);
    bool IsKeyContained(const string& key) const;
    void SetValue(double value);
    double GetValue() const;
    bool IsValid() const;

private:
    set<string> m_keys;
    double m_value;
};

class LengthConverter
{
public:
    LengthConverter(const vector<Rule>& defaultRules);
    ~LengthConverter();

    int Convert(const string& inputFilename,
                const string& outputFilename);

private:
    int Parse(const string& inputFilename);
    int ParseRule(const string& line,
                  size_t equalPos);
    int ParseLength(const string& sourceString,
                    size_t begin,
                    size_t end,
                    double& value,
                    string& key);
    bool IsMeterKey(const string& key);
    int Calculate();
    int CalculateInput(const string& input,
                       double& output);
    int ConvertToMeter(const string& sourceString,
                       size_t begin,
                       size_t end,
                       double& value);
    int WriteOutputs(const string& outputFilename);

    vector<Rule> m_rules;
    vector<string> m_inputs;
    vector<double> m_outputs;
};

int main()
{
    Rule mile;
    mile.AddKey("mile");
    mile.AddKey("miles");

    Rule yard;
    yard.AddKey("yard");
    yard.AddKey("yards");

    Rule inch;
    inch.AddKey("inch");
    inch.AddKey("inches");

    Rule foot;
    foot.AddKey("foot");
    foot.AddKey("feet");

    Rule fath;
    fath.AddKey("fath");
    fath.AddKey("faths");

    Rule furlong;
    furlong.AddKey("furlong");
    furlong.AddKey("furlongs");

    vector<Rule> rules;
    rules.push_back(mile);
    rules.push_back(yard);
    rules.push_back(inch);
    rules.push_back(foot);
    rules.push_back(fath);
    rules.push_back(furlong);

    LengthConverter converter(rules);
    int error = converter.Convert(INPUT_FILENAME, OUTPUT_FILENAME);

    if (error)
    {
        cout<<"Failed!\n";
    }
    else
    {
        cout<<"Succeeded, please check the file "<<OUTPUT_FILENAME<<"!\n";
    }

    return error;
}

Rule::Rule()
: m_keys(),
  m_value(0.0)
{
}

Rule::~Rule()
{
}

Rule::Rule(const Rule& anotherRule)
: m_keys(anotherRule.m_keys),
  m_value(anotherRule.m_value)
{
}

Rule&
Rule::operator=(const Rule& anotherRule)
{
    if (this == (&anotherRule))
    {
        return *this;
    }

    m_keys = anotherRule.m_keys;
    m_value = anotherRule.m_value;

    return *this;
}

void
Rule::AddKey(const string& key)
{
    m_keys.insert(key);
}

bool
Rule::IsKeyContained(const string& key) const
{
    return (m_keys.find(key) != m_keys.end());
}

bool
Rule::IsValid() const
{
    return (m_value > 0.0);
}

void
Rule::SetValue(double value)
{
    m_value = value;
}

double
Rule::GetValue() const
{
    return m_value;
}

LengthConverter::LengthConverter(const vector<Rule>& defaultRules)
: m_rules(defaultRules),
  m_inputs(),
  m_outputs()
{
}

LengthConverter::~LengthConverter()
{
}

int
LengthConverter::Convert(const string& inputFilename,
                         const string& outputFilename)
{
    if (inputFilename.empty() ||
        outputFilename.empty())
    {
        cout<<"Input or output filename is empty!\n";
        return 1;
    }

    int error = 0;

    cout<<"Parsing the input file...\n";
    error = Parse(inputFilename);
    if (error)
    {
        return 1;
    }

    cout<<"Calculating the results...\n";
    error = Calculate();
    if (error)
    {
        return 1;
    }

    cout<<"Writing the output file...\n";
    error = WriteOutputs(outputFilename);

    return error;
}

int
LengthConverter::Parse(const string& inputFilename)
{
    ifstream fs(inputFilename.c_str());
    if (!(fs.is_open()))
    {
        cout<<"Failed to open the input file!\n";
        return 1;
    }

    while (!(fs.eof()))
    {
        string line;
        getline(fs, line);

        if (line.empty())
        {
            continue;
        }

        // Find '=' to check if it is a rule.
        size_t equalPos = line.find("=");
        if (equalPos == string::npos)
        {
            m_inputs.push_back(line);
        }
        else
        {
            int error = ParseRule(line, equalPos);
            if (error)
            {
                cout<<"WARNING: there is an invalid rule \""<<line<<"\", it is ignored!\n";
            }
        }
    }

    fs.close();
    return 0;
}

int
LengthConverter::ParseRule(const string& line,
                           size_t equalPos)
{
    int error = 0;

    double value1 = 0.0;
    double value2 = 0.0;
    string key1;
    string key2;
    error = ParseLength(line, 0, equalPos, value1, key1);
    error = error ? error : ParseLength(line, equalPos + 1, line.length(), value2, key2);
    if (error)
    {
        return error;
    }

    double convertedValue = 0.0;
    string convertedKey;
    if (IsMeterKey(key1))
    {
        convertedKey = key2;

        // Check if the value of meters is about zero.
        if (value2 == 0.0) 
        {
            return 1;
        }

        convertedValue = value1 / value2;
    }
    else if (IsMeterKey(key2))
    {
        convertedKey = key1;

        // Check if the value of meters is about zero.
        if (value1 == 0.0)
        {
            return 1;
        }

        convertedValue = value2 / value1;
    }
    else
    {
        return 1;
    }

    vector<Rule>::iterator iter = m_rules.begin();
    vector<Rule>::const_iterator end = m_rules.end();
    for (; iter != end; ++iter)
    {
        if (iter->IsKeyContained(convertedKey))
        {
            iter->SetValue(convertedValue);
            break;
        }
    }

    if (iter == end)
    {
        return 1;
    }

    return 0;
}

int
LengthConverter::ParseLength(const string& sourceString,
                             size_t begin,
                             size_t end,
                             double& value,
                             string& key)
{
    size_t length = sourceString.length();
    end = (end <= length) ? end : length;

    if (begin >= end)
    {
        return 1;
    }

    stringstream ss(sourceString.substr(begin, end - begin));
    ss >> value >> key;

    // Convert the string of unit to lower cases.
    transform(key.begin(), key.end(), key.begin(), ::tolower);

    return 0;
}

bool
LengthConverter::IsMeterKey(const string& key)
{
    return ((key.compare("m") == 0) ||
            (key.compare("meter") == 0) ||
            (key.compare("meters") == 0));
}

int
LengthConverter::Calculate()
{
    vector<string>::const_iterator iter = m_inputs.begin();
    vector<string>::const_iterator end = m_inputs.end();
    for (; iter != end; ++iter)
    {
        const string& input = *iter;
        double output = 0.0;
        int error = CalculateInput(input, output);
        if (error)
        {
            cout<<"WARNING: Cannot calculate the line \""<<input<<"\", it is ignored!\n";
        }
        else
        {
            m_outputs.push_back(output);
        }
    }

    return 0;
}

int
LengthConverter::CalculateInput(const string& input,
                                double& output)
{
    if (input.empty())
    {
        return 0;
    }

    size_t length = input.length();
    double result = 0.0;
    int error = ConvertToMeter(input, 0, length, result);
    if (error)
    {
        return error;
    }

    size_t operatorPos = 0;
    size_t currentPos = 0;
    while ((operatorPos = input.find_first_of("+-", currentPos)) != string::npos)
    {
        if (operatorPos <= currentPos)
        {
            return 1;
        }

        double meter = 0.0;
        error = ConvertToMeter(input, operatorPos + 1, length, meter);
        if (error)
        {
            return error;
        }

        char operatorValue = input.at(operatorPos);
        if (operatorValue == '+')
        {
            result += meter;
        }
        else if (operatorValue == '-')
        {
            result -= meter;
        }
        else
        {
            return 1;
        }

        currentPos = operatorPos + 1;
    }

    output = result;
    return 0;
}

int
LengthConverter::ConvertToMeter(const string& sourceString,
                                size_t begin,
                                size_t end,
                                double& value)
{
    double result = 0.0;
    string key;
    int error = ParseLength(sourceString, begin, end, result, key);
    if (error)
    {
        return error;
    }

    if (!IsMeterKey(key))
    {
        vector<Rule>::const_iterator iter = m_rules.begin();
        vector<Rule>::const_iterator end = m_rules.end();
        for (; iter != end; ++iter)
        {
            if (iter->IsValid() && iter->IsKeyContained(key))
            {
                result *= iter->GetValue();
                break;
            }
        }

        if (iter == end)
        {
            return 1;
        }
    }

    value = result;
    return 0;
}

int
LengthConverter::WriteOutputs(const string& outputFilename)
{
    remove(OUTPUT_FILENAME);
    ofstream fs(OUTPUT_FILENAME);
    if (!(fs.is_open()))
    {
        cout<<"Failed to open the output file!\n";
        return 1;
    }
  
    fs<<"asongala@gmail.com\r\n\r\n";

    vector<double>::const_iterator iter = m_outputs.begin();
    vector<double>::const_iterator end = m_outputs.end();
    for (; iter != end; ++iter)
    {
        fs<<fixed<<setprecision(2)<<*iter<<" m\r\n";
    }

    fs.close();
    return 0;
}

