#ifndef JSON_H
    #include "JSON.h"
#endif


using namespace std;

namespace JsonMaker{

	//utils functions
    string getOnly(string source, string allowed)
    {
        stringstream ret;
        for (const auto& curr : source)
        {
            if (allowed.find(curr) != string::npos)
                ret << curr;
        }

        return ret.str();
    }

    std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace) {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }

    map<string, IJSONObject*>::const_iterator getChildByIndex (map<string, IJSONObject*> *maptosearch, int index, bool *sucess){
        map<string, IJSONObject*>::const_iterator end = (*maptosearch).end();

        int counter = 0;
        for (map<string, IJSONObject*>::const_iterator it = (*maptosearch).begin(); it != end; ++it) {

            if (counter == index)
            {
                *sucess = true;
                return it;
            }
            counter++;
        }

        *sucess = false;

        return {};
    }

    // trim from start (in place)
    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    // trim from end (in place)
    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    }

    // trim from both ends (in place)
    inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    // trim from start (copying)
    inline std::string ltrim_copy(std::string s) {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    inline std::string rtrim_copy(std::string s) {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    inline std::string trim_copy(std::string s) {
        trim(s);
        return s;
    }

    string __unescapeString(string data)
    {
        //result = result.Replace("\\\\", "\\").Replace("\\\"", "\"").Replace("\\r", "\r").Replace("\\n", "\n").Replace("\\t", "\t");
        string nValue = "";
        unsigned int cont;
        for (cont = 0; cont < data.size() - 1; cont++)
        {
            if (data[cont] == '\\')
            {
                if (data[cont + 1] == '\"')
                    nValue += '\"';
                else if (data[cont + 1] == '\r')
                    nValue += '\r';
                else if (data[cont + 1] == '\n')
                    nValue += '\n';
                else if (data[cont + 1] == '\t')
                    nValue += '\t';
                else if (data[cont + 1] == '\\')
                    nValue += '\\';
                else
                    nValue += '?';

                cont++;
            }
            else
                nValue += data[cont];

            //cont++;


        }
        if (cont < data.size())
            nValue = nValue + data[cont];

        return nValue;
    }
}

namespace JsonMaker{

	//IJSONObject


    bool IJSONObject::isArray()
    {




		vector<string> childsNames = this->__getChildsNames();
		if (childsNames.size() == 0)
			return false;

        unsigned int cont = 0;
        while (cont < childsNames.size())
        {
            if (getOnly(childsNames[cont], "0123456789") != childsNames[cont])
                return false;
            cont++;
        }
        return childsNames.size() > 0;
    }

	void IJSONObject::forceType(SOType forcedType)
	{
		this->forcedType = forcedType;
	}

	string IJSONObject::ToJson(bool quotesOnNames, bool format, int level)
    {
        stringstream result;


		vector<string> childsNames = this->__getChildsNames();

        if (childsNames.size() > 0)
        {
            bool array = this->isArray();
            if (array)
                result << "[";
            else
                result << "{";


            if (format)
                result << "\r\n";

            level++;

            for (unsigned int cont = 0; cont < childsNames.size(); cont++)
            {
                if (format)
                    for (int a = 0; a < level; a++)
                        result << "    ";

                auto current = this->__getChild(childsNames[cont]);
                if (array)
				{
					if ((current != NULL))
						result << current->ToJson(quotesOnNames, format, level);
				}
				else
				{
					if (quotesOnNames)
						result << '"' + childsNames[cont] + "\":" + current->ToJson(quotesOnNames, format, level);
					else
						result << childsNames[cont] + ":" + current->ToJson(quotesOnNames, format, level);
				}

                if (cont < childsNames.size() - 1)
                {
                    result << ",";
                    if (format)
                        result << "\r\n";
                }
            }

            level --;
            if (format)
            {
                result << "\r\n";
                for (int a = 0; a < level; a++)
                    result << "    ";
            }

            if (array)
                result << "]";
            else
                result << "}";


            return result.str();
        }
        else
        {
            return serializeSingleValue();
        }
    }


	SOType IJSONObject::__determineSoType(string value)
	{
        //trye as null
        if ((value == "null") || (value == ""))
            return SOType::Null;
        else
        {
            //try as boolean
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);

            if ((value == "true") || (value == "false"))
                return SOType::Boolean;
            else
            {
                //try as int
                if (getOnly(value, "0123456789") == value)
                    return SOType::Int;
                else
                {
					//try as datetime
					if (getOnly(value, "0123456789:/-+TtZz") == value)
						return SOType::DateTime;
                    //try as double
                    else{
						if (getOnly(value, "0123456789.") == value)
							return SOType::Double;
						else
						{
							//is a string
							return SOType::String;
						}
					}
                }
            }
        }

	}
}

namespace JsonMaker{
	//InMemoryJsonObject
	void InMemoryJsonObject::Initialize(IJSONObject *pParent, string relativeName, IJSONObject *modelObject)
	{
		this->relativeName = relativeName;
        this->parent = pParent;
    }

	void InMemoryJsonObject::setChild(string name, IJSONObject *child)
    {
        this->childs[name] = child;
    }

	void InMemoryJsonObject::del(string name)
    {
        this->childs.erase(name);
    }

	void InMemoryJsonObject::clear()
    {
        for (const auto& curr : this->childs) {
            curr.second->clear();
            delete curr.second;
        }
        this->singleValue = "";
        this->relativeName = "";
        this->childs.clear();
    }

    string InMemoryJsonObject::serializeSingleValue()
    {
		if (this->type == SOType::Null)
			return "null";
		else if (this->type == SOType::Boolean)
		{
			string temp = this->singleValue;
			std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

			temp =  ((temp == "true") || (temp == "1")) ? "true" : "false";
			return temp;
		}
		else if (this->type == SOType::String)
		{
			if (this->singleValue.size() > 0)
				if (this->singleValue[0] != '"')
					return '"' + this->singleValue + '"';
				else
					return this->singleValue;
			else
				return "\"\"";
		}
		else if (this->type == SOType::Double)
		{
			std::replace( this->singleValue.begin(), this->singleValue.end(), ',', '.'); // replace all 'x' to 'y'
			return this->singleValue;
		}
		else
			return this->singleValue;
    }

	SOType InMemoryJsonObject::getJSONType()
    {
        if (this->childs.size() > 0)
		{
			if (this->isArray())
				return SOType::__Array;
			else
				return SOType::__Object;
		}
		return this->type;
    }

    void InMemoryJsonObject::setSingleValue(string value, SOType forceType)
    {
        if (forceType == SOType::Undefined)
            this->type = this->__determineSoType(value);
        else
            this->type = forceType;

		if (this->type != SOType::Null)
			this->singleValue = value;
    }

	vector<string> InMemoryJsonObject::__getChildsNames()
	{
		vector<string> result;
		bool sucess;
		for (unsigned int cont = 0; cont < this->childs.size(); cont++)
		{
			auto current = getChildByIndex(&(this->childs), cont, &sucess);
			if (sucess)
				result.push_back(current->first);
		}

		return result;
	}

	IJSONObject* InMemoryJsonObject::__getChild(string name, bool caseSensitive)
    {
        bool sucess;
        if (caseSensitive)
		{
			if (this->childs.find(name) != this->childs.end())
				return this->childs[name];
		}
		else{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			for (unsigned int cont = 0; cont < this->childs.size(); cont++)
			{
				auto current = getChildByIndex(&(this->childs), cont, &sucess);
				if (!sucess)
                    return NULL;
				string currName = current-> first;
				std::transform(currName.begin(), currName.end(), currName.begin(), ::tolower);
				if (name == currName)
					return current->second;
			}
		}

		return NULL;
    }

	bool InMemoryJsonObject::__containsChild(string name, bool caseSensitive)
	{
        bool sucess;
		if (caseSensitive)
		{
			if (this->childs.find(name) != this->childs.end())
				return true;
			else
				return false;
		}
		else{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			for (unsigned int cont = 0; cont < this->childs.size(); cont++)
			{
				auto current = getChildByIndex(&(this->childs), cont, &sucess);
				if (!sucess)
                    return false;

				string currName = current-> first;
				std::transform(currName.begin(), currName.end(), currName.begin(), ::tolower);
				if (name == currName)
					return true;
			}

			return false;

		}
	}

	string InMemoryJsonObject::getRelativeName()
	{
		return this->relativeName;
	}

	bool InMemoryJsonObject::isDeletable()
	{
		return false;
	}
}

namespace JsonMaker{

	//the json library
    void JSON::clear()
    {
        if (this->root != NULL)
        {
            this->root->clear();
        }

        if (modelObject != NULL)
        {
            modelObject->clear();
        }

    }

    void JSON::clearAndFree()
    {
        if (this->root != NULL)
        {
            this->root->clear();
            delete this->root;
            this->root = NULL;
        }

        if (modelObject != NULL)
        {
            modelObject->clear();
            delete modelObject;
            modelObject = NULL;
        }

    }

	void JSON::internalInitialize(IJSONObject* _modelObject)
	{
		if ( _modelObject == NULL)
			_modelObject = new InMemoryJsonObject();

		this->modelObject = _modelObject;

        //typeid
		if (dynamic_cast<InMemoryJsonObject*>(modelObject))
			root = new InMemoryJsonObject();

		root->Initialize(NULL, "", this->modelObject);
	}


	JSON::JSON(bool caseSensitiveToFind, IJSONObject *_modelObject)
	{
		this->caseSensitiveToFind = caseSensitiveToFind;
		this->internalInitialize(_modelObject);
	}

	JSON::JSON(string JsonString, bool caseSensitiveToFind, IJSONObject *_modelObject)
	{
		this->caseSensitiveToFind = caseSensitiveToFind;
		this->internalInitialize(_modelObject);
		this->parseJson(JsonString);
	}

	JSON::~JSON()
	{
        this->clearAndFree();
	}

    IJSONObject *JSON::find(string objectName, bool autoCreateTree, IJSONObject *currentParent, SOType forceType)
    {
		//quebra o nome em um array
		objectName = ReplaceString(objectName, "]", "");
        objectName = ReplaceString(objectName, "[", ".");

		//remove '.' from start (like when lib is used with json.set("[0].foo")
		while (objectName != "" && objectName[0] == '.')
			objectName = objectName.substr(1);

		std::string currentName = objectName;
        std::string childsNames = "";
        IJSONObject *childOnParent;

		if (objectName.find(".") != string::npos)
        {
            currentName = objectName.substr(0, objectName.find("."));
            childsNames = objectName.substr(objectName.find(".") + 1);
        }

		if (!currentParent->__containsChild(currentName, this->caseSensitiveToFind))
        {
            if (autoCreateTree)
            {
				IJSONObject *tempObj = NULL;
				string currentParentRelativeName = currentParent->getRelativeName();

				if (dynamic_cast<InMemoryJsonObject*>(currentParent))
					tempObj = new InMemoryJsonObject();

				tempObj->Initialize((InMemoryJsonObject*)currentParent, currentParent->getRelativeName(), this->modelObject);

				if (forceType != SOType::Undefined)
					tempObj->forceType(forceType);



				currentParent->setChild(currentName, tempObj);
				tempObj = NULL;
            }
            else
                return NULL;
        }


		childOnParent = currentParent->__getChild(currentName, this->caseSensitiveToFind);


		if (childsNames == "")
		{
			return childOnParent;
		}
		else
		{
			return this->find(childsNames, autoCreateTree, childOnParent);
		}
    }

    void JSON::_set(string objectName, string value)
    {

        if (this->isAJson(value))
        {
            this->parseJson(value, objectName);
            return;
        }

        IJSONObject* temp = this->find(objectName, true, this->root);


        /*if (value[0] == '\"')
            value = value.Substring(1, value.Length - 2);*/

        //value = value.Replace("\"", "\\\"");
        temp->setSingleValue(value);

    }

    void JSON::del(IJSONObject *node)
    {
		auto childs = node->__getChildsNames();
		for (const auto& c: childs)
		{
			del(node->__getChild(c));
		}
		node->clear();
		childs.clear();


        /*auto childsNames = node->__getChildsNames();
        for (const auto& c : childsNames)
        {
            del(node->__getChild(c);
        }
        childs->clear();*/

        /*auto parentNodes = node->parent->__getChilds();
        for (int cont = 0; cont < parentNodes->size(); cont++)
        {
            auto find = getChildByIndex(parentNodes, cont, &sucess);
            if (sucess && find->second == node)
            {
                //if parent is an array, pull the elements forward backwards
                if (node->parent->isArray())
                {
                    for (int cont2 = cont; cont2 < parentNodes->size() - 1; cont2++)
                        (*parentNodes)[getChildByIndex(parentNodes, cont2, &sucess)->first] = (*parentNodes)[getChildByIndex(parentNodes, cont2 + 1, &sucess)->first];

                    parentNodes->erase(parentNodes->rend()->first);
                }
                else
                {
                    parentNodes->erase(parentNodes->rend()->first);
                }
                break;
            }
        }*/
    }

	/// <summary>
    /// Removes an object from JSON three
    /// </summary>
    /// <param name="objectName">The object name</param>
    void JSON::del(string objectName)
    {
        IJSONObject *temp = this->find(objectName, false, this->root);
        if (temp != NULL)
            del(temp);
    }

	void JSON::clearChilds(string objectName)
    {
        IJSONObject *temp = this->find(objectName, false, this->root);

        if (temp != NULL)
        {
            //auto childs = temp->__getChilds();
			auto names = temp->__getChildsNames();
            for (const auto& c : names)
            {
                del(temp->__getChild(c));
            }
        }
    }

	/// <summary>
    /// Insert a new json in current json three
    /// </summary>
    /// <param name="objectName">Name of the object</param>
    /// <param name="toImport">Json to be imported</param>
    void JSON::set(string objectName, JSON *toImport)
    {

		if (objectName != "")
		{
			if (!objectName.find("\"") == 0)
				objectName = '"' + objectName + '"';
			objectName = "{" + objectName + ":" + toImport->ToJson() + "}";
			this->parseJson(objectName, "");
		}
		else
		{
			this->parseJson(toImport->ToJson());
		}

    }


	/// <summary>
    /// Set or creates an property with an json string
    /// </summary>
    /// <param name="objectName">The json object name</param>
    /// <param name="value">The json string </param>

	void JSON::set(string objectName, string value, SOType forceType)
	{
		if (forceType == SOType::Undefined && isAJson(value))
		{
			this->parseJson(value, objectName);
		}
		else
		{
			auto found = this->find(objectName, true, this->root, forceType);
			found->setSingleValue(value);
		}

	}

	/// <summary>
    /// Serialize the Json three
    /// </summary>
    /// <param name="quotesOnNames">User '"' in name of objects</param>
    /// <returns></returns>
    string JSON::ToJson(bool format)
    {
        std::string result = this->root->ToJson(true, format);
        return result;
    }

    string JSON::ToString()
    {
        return this->ToJson();
    }

	/// <summary>
    /// Return true if the an object is in json three
    /// </summary>
    /// <param name="objectName">The object name</param>
    /// <returns></returns>
    bool JSON::contains(string objectName)
    {
        bool result = this->find(objectName, false, this->root) != NULL;
        return result;
    }

    /// <summary>
    /// returns the value of an json object as a json string (Serialize an object)
    /// </summary>
    /// <param name="objectName">The object name</param>
    /// <param name="quotesOnNames">User '"' in names</param>
    /// <returns></returns>
    string JSON::get(string objectName, bool format, bool quotesOnNames, string valueOnNotFound)
    {
        IJSONObject *temp = this->find(objectName, false, this->root);
        if (temp != NULL)
            return temp->ToJson(quotesOnNames, format);
        else
            return valueOnNotFound;

    }

	IJSONObject *JSON::getRaw(string objectName)
	{
		return this->find(objectName, false, this->root);
	}


    vector<string> JSON::getObjectsNames(IJSONObject *currentItem)
    {
        vector<string> retorno;

        if (currentItem == NULL)
            currentItem = this->root;


        string parentName = "";

        vector<string> childsNames;


		auto childsNamesList = currentItem->__getChildsNames();
        for (unsigned int cont = 0; cont < childsNamesList.size(); cont++)
        {

            childsNames = getObjectsNames(currentItem->__getChild(childsNamesList[cont]));


            parentName = childsNamesList[cont];
            //adiciona os filhos ao resultado
            //verifica se o nome atual atende ao filtro
            for (const auto& att : childsNames)
            {
                string nAtt = att;
                if (nAtt != "")
                    nAtt = parentName + '.' + nAtt;

                retorno.push_back(nAtt);
            }
            retorno.push_back(childsNamesList[cont]);
        }
        return retorno;

    }

	/// <summary>
    /// Return all names of the json three of an object
    /// </summary>
    /// <param name="objectName">The name of object</param>
    /// <returns></returns>
    vector<string> JSON::getObjectsNames(string objectName)
    {
        if (objectName == "")
        {
            IJSONObject *nullo = NULL;
            return this->getObjectsNames(nullo);
        }
        else
        {
            IJSONObject* finded = this->find(objectName, false, this->root);
            vector<string> retorno;
            if (finded != NULL)
                retorno = this->getObjectsNames(finded);

            return retorno;
        }
    }

	vector<string> JSON::getChildsNames(IJSONObject *currentItem)
    {
        vector<string> retorno;


        if (currentItem == NULL)
            currentItem = this->root;

		auto childsNames = currentItem->__getChildsNames();

        for (unsigned int cont = 0; cont < childsNames.size(); cont++)
        {
            retorno.push_back(childsNames[cont]);
        }
        return retorno;
    }

    /// <summary>
    /// Return the childNames of an json object
    /// </summary>
    /// <param name="objectName">The name of object</param>
    /// <returns></returns>
    vector<string> JSON::getChildsNames(string objectName)
    {
        if (objectName == "")
        {
            IJSONObject *nullo = NULL;
            return this->getChildsNames(nullo);
        }
        else
        {
            IJSONObject* finded = this->find(objectName, false, this->root);
            vector<string> retorno;
            if (finded != NULL)
                retorno = this->getChildsNames(finded);
            return retorno;
        }

    }


    void JSON::fromJson(string json, bool tryParseInvalidJson)
	{
		this->parseJson(json, "", tryParseInvalidJson, SOType::Undefined);
	}

	void JSON::fromString(string json, bool tryParseInvalidJson)
	{
		this->parseJson(json, "", tryParseInvalidJson, SOType::Undefined);

	}

	enum ParseStates { findingStart, readingName, waitingKeyValueSep, findValueStart, prepareArray, readingContentString, readingContentNumber, readingContentSpecialWord };
	void JSON::parseJson(string json, string parentName, bool tryParseInvalidJson, SOType forceType)
	{
        if (json == "null")
        {
            return;
        }

		auto currentObject = this->root;

		if (parentName != "")
			currentObject = this->find(parentName, true, root);

		currentObject->name = parentName;

		ParseStates state = ParseStates::findValueStart;

		bool ignoreNextChar = false;
		stringstream currentStringContent;
		stringstream currentNumberContent;
		stringstream currentSpecialWordContent;
		stringstream currentChildName;

		int currLine = 1;
		int currCol = 1;

		int max = json.size();
		char curr = ' ';

		for (int cont = 0; cont < max; cont++)
		{
			curr = json[cont];

			currCol++;
			if (curr == '\n')
			{
				currCol = 1;
				currLine++;
			}

			switch (state)
			{
				case ParseStates::findingStart:
					if (curr == '"')
					{
						if (currentObject->isArray())
							state = ParseStates::prepareArray;
						else
							state = ParseStates::readingName;
						currentChildName.str("");
					}
					else if ((curr == ',')/* || (curr == '[') || (curr == '{')*/)
					{
						if (currentObject->isArray())
							state = ParseStates::prepareArray;
					}

					else if ((curr == '}') || (curr == ']'))
					{
						if (parentName.find('.') != string::npos)
						{
							parentName = parentName.substr(0, parentName.find_last_of('.'));
							if (currentObject != NULL)
								currentObject = currentObject->parent;
						}
						else
						{
							parentName = "";
							currentObject = root;
						}
					}
					break;
				case ParseStates::readingName:
					if (curr == '"')
					{
						state = ParseStates::waitingKeyValueSep;
						currentObject = this->find(currentChildName.str(), true, currentObject, forceType);
						currentObject->name = currentChildName.str();
						parentName = parentName + (parentName != "" ? "." : "") + currentChildName.str();
						currentChildName.str("");

					}
					else
						currentChildName << curr;
					break;
				case ParseStates::waitingKeyValueSep:
					if (curr == ':')
						state = ParseStates::findValueStart;
					break;
				case ParseStates::findValueStart:
					if (curr == '"')
					{
						state = ParseStates::readingContentString;
						currentStringContent.str("");
					}
					else if (curr == '{')
					{
						state = ParseStates::findingStart;
					}
					else if (curr == '[')
						state = ParseStates::prepareArray;
					else if (string("0123456789-+.").find(curr) != string::npos)
					{
						state = ParseStates::readingContentNumber;
						currentNumberContent.str("");
						cont--;
						currCol--;
					}
					else if (string("untfUNTF").find(curr) != string::npos)
					{
						state = ParseStates::readingContentSpecialWord;
						currentSpecialWordContent.str("");
						cont--;
						currCol--;
					}
					else if (curr == ']')
					{
						//delete currenObject
						auto temp = currentObject;


						if (parentName.find('.') != string::npos)
						{
							parentName = parentName.substr(0, parentName.find_last_of('.'));
							currentObject = currentObject->parent;
						}
						else
						{
							parentName = "";
							currentObject = root;
						}

						currentObject->del(temp->name);

						cont--;
						currCol--;
						state = ParseStates::findingStart;
					}
					else if (string(" \t\r\n").find(curr) == string::npos)
					{
                        string errorMessage = "SintaxError at line "+to_string(currLine) + " and column "+to_string(currCol) + ". Expected ' '(space), \t, \r or \n, but found "+curr+".";
						if(!tryParseInvalidJson)
							throw errorMessage;
					}
					break;

				case ParseStates::prepareArray:
					//state = "findingStart";
					currentChildName.str("");
					currentChildName << to_string(currentObject->__getChildsNames().size());
					currentObject = this->find(currentChildName.str(), true, currentObject, forceType);
					currentObject->name = currentChildName.str();
					parentName = parentName + (parentName != "" ? "." : "") + currentChildName.str();
					state = ParseStates::findValueStart;
					cont--;
					currCol--;
					break;
				case ParseStates::readingContentString:
					if (ignoreNextChar)
					{
						currentStringContent << curr;
						ignoreNextChar = false;
					}
					else if (curr == '\\')
					{
						ignoreNextChar = true;
						currentStringContent << curr;
					}
					else if (curr == '"')
					{
						currentObject->setSingleValue(currentStringContent.str(), SOType::String);
						currentStringContent.str("");

						//return to parent Object
						if (parentName.find('.') != string::npos)
						{
							parentName = parentName.substr(0, parentName.find_last_of('.'));
							currentObject = currentObject->parent;
						}
						else
						{
							parentName = "";
							currentObject = root;
						}

						state = ParseStates::findingStart;

					}
					else
						currentStringContent << curr;
					break;
				case ParseStates::readingContentNumber:
					if (string("0123456789.-+").find(curr) != string::npos)
						currentNumberContent << curr;
					else
					{
						currentObject->setSingleValue(currentNumberContent.str());
						currentNumberContent.str("");

						//return to parent Object
						if (parentName.find('.') != string::npos)
						{
							parentName = parentName.substr(0, parentName.find_last_of('.'));
							currentObject = currentObject->parent;
						}
						else
						{
							parentName = "";
							currentObject = root;
						}

						cont--;
						state = ParseStates::findingStart;
					}

					break;
				case ParseStates::readingContentSpecialWord:
					if (string("truefalseundefinednulTRUEFALSEUNDEFINEDNUL").find(curr) != string::npos)
						currentSpecialWordContent<<curr;
					else
					{
                        string strTemp = currentSpecialWordContent.str();
                        std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
						if ((strTemp == "true") ||
							(strTemp == "false") ||
							(strTemp == "null") ||
							(strTemp == "undefined"))
						{
							currentObject->setSingleValue(strTemp);
							currentSpecialWordContent.str("");

							//return to parent Object
							if (parentName.find('.') != string::npos)
							{
								parentName = parentName.substr(0, parentName.find_last_of('.'));
								currentObject = currentObject->parent;
							}
							else
							{
								parentName = "";
								currentObject = root;
							}

							cont--;
							state = ParseStates::findingStart;
						}
						else
						{
                            string errorMessage = "Invalid simbol at line " + to_string(currLine) + " and column " + to_string(currCol) + ": " + currentSpecialWordContent.str();
							if (!tryParseInvalidJson)
								throw errorMessage;
						}
					}

					break;
			}
		}
	}

    vector<string> JSON::getJsonFields(string json)
    {
        int open = 0;
        vector<string> fields;
        stringstream temp;
        bool quotes = false;
		bool skeepNext = false;

        for (unsigned int cont = 1; cont < json.size() - 1; cont++)
        {
			if (skeepNext)
			{
				temp << json[cont];
				skeepNext = false;
				continue;
			}

			if (!quotes)
			{
				if (json[cont] == ',')
				{
					if (open == 0)
					{
						fields.push_back(temp.str());
						temp.str("");
						continue;
					}
				}
				else if ((json[cont] == '{') || (json[cont] == '['))
					open++;
				else if ((json[cont] == '}') || (json[cont] == ']'))
					open--;
			}
			else
			{
				if (json[cont] == '\\')
				{
					temp << json[cont];
					skeepNext = true;
					continue;
				}
			}

			if (json[cont] == '"')
			{
				//if ((json[cont - 1] != '\\') || (json[cont - 2] == '\\'))
				quotes = !quotes;
			}

			// if ((quotes) || (temp.Length == 0) || (!"}]".Contains(temp[temp.Length - 1])))
			temp << json[cont];

        }

        //get size of temp
        temp.seekg(0, ios::end);
        int size = temp.tellg();
        if (size > 0)
            fields.push_back(temp.str());

        return fields;

    }

    string JSON::clearJsonString(string json)
    {
		stringstream result;

		bool quotes = false;
		//char oldOldAtt = ' ';
		//char oldAtt = ' ';
		bool skeepNext = false;
		for (const auto& att : json)
		{
			if (skeepNext)
			{
				result << att;
				skeepNext = false;
				continue;
			}

			if (att == '\"')
				quotes = !quotes;

			if (!quotes)
			{
				if (string("\r\n\t\0 ").find(att) == string::npos)
					result << att;
			}
			else
			{
				if (att == '\\')
				{
					skeepNext = true;
				}

				result << att;
			}

			//oldOldAtt = oldAtt;
			//oldAtt = att;
		}
		return result.str();
    }

    bool JSON::isAJson(string json, bool objects, bool arrays)
    {
        bool quotes = false;

        char oldAtt = (char)0;
        int cont = 0;
        json = ltrim_copy(json);

        string objectkeys = "{}";
        string vectorkeys = "[]";
        for (const auto& att : json)
        {
            cont++;
            if ((att == '\"') && (oldAtt != '\\'))
                quotes = !quotes;

            if (!quotes)
            {
                if (att == ':')
                    return true;
                if ((objects) && (objectkeys.find(att) != string::npos) && (cont == 0))
                    return true;
                else if ((arrays) && (vectorkeys.find(att) != string::npos) && (cont == 0))
                    return true;
            }
            oldAtt = att;
        }

        return false;
    }





    SOType JSON::getJSONType(string objectName)
	{
		IJSONObject *temp = this->find(objectName, false, this->root);
		if (temp != NULL)
		{
			return temp->getJSONType();
		}
		else
			return SOType::Undefined;
	}


    /// <summary>
    /// Get a json property as string
    /// </summary>
    /// <param name="name">Object name of the property</param>
    /// <param name="defaultValue">Value to be returned when the property is not found</param>
    /// <returns></returns>
    string JSON::getString(string name, string defaultValue)
    {
        string result = this->get(name);


        if ((result.size() > 0) && (result[0] == '"'))
            result = result.substr(1);
        if ((result.size() > 0) && (result[result.size() - 1] == '"'))
            result = result.substr(0, result.size() - 1);

		result = __unescapeString(result);

        if ((result != "") && (result != "undefined"))
            return result;
        else
            return defaultValue;

    }

    /// <summary>
    /// Set or create a property as string
    /// </summary>
    /// <param name="name">The property object name </param>
    /// <param name="value">The value</param>
    void JSON::setString(string name, string value, bool tryRedefineType)
    {
		if (tryRedefineType && isAJson(value))
		{
			this->parseJson(value, name);
        }
		else{

            value = ReplaceString(value, "\\", "\\\\");
            value = ReplaceString(value, "\"", "\\\"");
            value = ReplaceString(value, "\r", "\\r");
            value = ReplaceString(value, "\n", "\\n");
            value = ReplaceString(value, "\t", "\\t");

            this->set(name, '"' + value + '"', tryRedefineType ? SOType::Undefined : SOType::String);
		}
    }

    /// <summary>
    /// Get a json property as int
    /// </summary>
    /// <param name="name">Object name of the property</param>
    /// <param name="defaultValue">Value to be returned when the property is not found</param>
    /// <returns></returns>
    int JSON::getInt(string name, int defaultValue)
    {
        string temp = getOnly(this->get(name), "0123456789-");
        if (temp != "")
            return std::stoi(temp);
        else return defaultValue;
    }

    /// <summary>
    /// Set or create a property as int
    /// </summary>
    /// <param name="name">The property object name </param>
    /// <param name="value">The value</param>
    void JSON::setInt(string name, int value)
    {
        this->set(name, std::to_string(value), SOType::Int);
    }

    /// <summary>
    /// Get a json property as Int64
    /// </summary>
    /// <param name="name">Object name of the property</param>
    /// <param name="defaultValue">Value to be returned when the property is not found</param>
    /// <returns></returns>
    long JSON::getLong(string name, long defaultValue)
    {
        string temp = getOnly(this->get(name), "0123456789-");
        if (temp != "")
            return std::stol(temp);
        else return defaultValue;
    }

    /// <summary>
    /// Set or create a property as int64
    /// </summary>
    /// <param name="name">The property object name </param>
    /// <param name="value">The value</param>
    void JSON::setLong(string name, long value)
    {
        this->set(name, std::to_string(value), SOType::Int);
    }

    /// <summary>
    /// Get a json property as boolean
    /// </summary>
    /// <param name="name">Object name of the property</param>
    /// <param name="defaultValue">Value to be returned when the property is not found</param>
    /// <returns></returns>
    bool JSON::getBoolean(string name, bool defaultValue)
    {
        string temp = this->get(name);
        if (temp != "")
        {
			std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
            if ((temp == "true") || (temp == "1"))
                return true;
            else
                return false;
        }
        else return defaultValue;
    }

    /// <summary>
    /// Set or create a property as boolean
    /// </summary>
    /// <param name="name">The property object name </param>
    /// <param name="value">The value</param>
    void JSON::setBoolean(string name, bool value)
    {
		//std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        this->set(name, value ? "true" : "false", SOType::Boolean);
    }

    /// <summary>
    /// Get a json property as double. To get a custom DateTime, please use getString
    /// </summary>
    /// <param name="name">Object name of the property</param>
    /// <param name="defaultValue">Value to be returned when the property is not found</param>
    /// <returns></returns>
    double JSON::getDouble(string name, double defaultValue)
    {
        //string temp = getOnly(this->get(name).Replace('.', ','), "0123456789-,");
        string temp = getOnly(this->get(name), "0123456789-,.");
        if (temp != "")
            return std::stod(temp);
        else return defaultValue;
    }

    /// <summary>
    /// Set or create a property as Double
    /// </summary>
    /// <param name="name">The property object name </param>
    /// <param name="value">The value</param>
    void JSON::setDouble(string name, double value)
    {
        string temp = std::to_string(value);
        temp = ReplaceString(temp, ",", ".");
        this->set(name, temp);
    }

    /// <summary>
    /// Return the childs count of an object (like arrays or objects)
    /// </summary>
    /// <param name="objectName">The name of the object</param>
    /// <returns></returns>
    int JSON::getArrayLength(string objectName)
    {
        IJSONObject* finded = this->find(objectName, false, this->root);

        if (finded != NULL)
            return finded->__getChildsNames().size();
        return 0;
    }

    void JSON::Dispose()
    {
        this->clearAndFree();
    }

}
