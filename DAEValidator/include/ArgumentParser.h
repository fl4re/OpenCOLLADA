#pragma once

#include "no_warning_functional"
#include "no_warning_map"
#include <sstream>
#include "no_warning_string"
#include "no_warning_vector"

namespace opencollada
{
	class Argument
	{
		friend class ArgumentParser;
	public:
		Argument& numParameters(size_t n); // Number of parameter for this argument.
		Argument& required(); // Sets this argument as required.
		Argument& hint(const std::string & s); // Argument hint.
		Argument& hint(size_t index, const std::string & s); // Argument parameter hint.
		Argument& help(const std::string & s); // Argument help.
		Argument& flags(size_t flags);

		size_t getNumParameters() const;
		size_t getFlags() const;
		bool isSet() const;
		bool isRequired() const;
		std::string getHint() const;
		std::string getHint(size_t index) const;
		std::string getHelp() const;

		template<typename T>
		T getValue(size_t index = 0) const
		{
			T value;
			std::stringstream ss;
			ss << mValues[index];
			ss >> value;
			return value;
		}

		operator bool() const;

	private:
		void isSet(bool is_set);
		void setValue(const std::string & str, size_t index = 0);

	private:
		static Argument null;
		std::vector<std::string> mValues;
		std::string mHint;
		std::string mHelp;
		std::vector<std::string> mHints;
		size_t mFlags = 0;
		bool mSet = false;
		bool mRequired = false;
	};
    
    template<>
    inline std::string Argument::getValue<std::string>(size_t index) const
    {
        return mValues[index];
    }

	class ArgumentParser
	{
	public:
		ArgumentParser(int argc, char* argv[]);

		bool parseArguments();
		std::string getParseError() const;

		Argument& addArgument(const std::string & name = "");
		const Argument& findArgument(const std::string & name);
		const Argument& findArgument(size_t index);
		size_t numSetArguments() const;
		std::string usage() const;
		bool hasSetArgument(size_t flags) const;

	private:
		std::map<std::string, Argument> mArguments;
		std::vector<Argument> mNoSwitchArguments;
		std::vector<std::string> mCommandLine;
		std::string mParseError;
	};
}