// $(header)
#include <oooii/oTest.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oXML.h>
#include <oooii/oRef.h>
#include <oooii/oBuffer.h>
#include <oooii/oFile.h>

class EventXMLMerger
{
	struct XMLandNode
	{
		threadsafe oXML* xml;
		oXML::HNODE node;
	};

	typedef std::vector<XMLandNode> tNodes;
	tNodes nodes;

public:
	void LoadXML(const char* FileName)
	{
		char xmlPath[_MAX_PATH];
		if(!oTestManager::Singleton()->FindFullPath(xmlPath, FileName))
			return;

		XMLandNode xandn;
		{
			char* pBuffer = 0;
			size_t size = 0;
			oFile::LoadBuffer((void**)&pBuffer, &size, malloc, xmlPath, true);
			oXML::Create(xmlPath, pBuffer, 50, 50, &xandn.xml);
			free(pBuffer);
		}

		oXML::HNODE rootNode = xandn.xml->GetFirstChild(0, "Stats");
		if(rootNode)
		{
			xandn.node = xandn.xml->GetFirstChild(rootNode, "Event");

			if(xandn.node)
				nodes.push_back(xandn);
		}
	}

	bool GetNextEventNode(threadsafe oXML** _ppXML, oXML::HNODE* _pNode)
	{
		tNodes::iterator itEarliest = nodes.end();
		oDateTime dateTimeEarliest;

		for(tNodes::iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			const char* timeStamp = (*it).xml->FindAttributeValue((*it).node, "Timestamp");
			oDateTime dateTime;
			oFromString(&dateTime, timeStamp);

			if(itEarliest == nodes.end() || oCompareDateTime(dateTimeEarliest, dateTime) > 0)
			{
				dateTimeEarliest = dateTime;
				itEarliest = it;
			}
		}

		if(itEarliest == nodes.end())
		{
			// No more events.
			return false;
		}

		*_ppXML = (*itEarliest).xml;
		*_pNode = (*itEarliest).node;

		(*itEarliest).node = (*itEarliest).xml->GetNextSibling((*itEarliest).node, "Event");

		if(!(*itEarliest).node)
		{
			// Last event for this file.
			nodes.erase(itEarliest);
		}

		return true;
	}
};

struct TESTStatParse : public oTest
{
	struct User
	{
		int ID;
		std::string StartTime;
		std::string EndTime;
		std::string Score;
		int Swings;
		int Hits;
		bool Player;
		int Spectators;

		User()
			: ID(-1)
			, Swings(0)
			, Hits(0)
			, Player(false)
			, Spectators(0)
		{ }
	};

	typedef std::vector<User> tUserList;
	std::vector<User> UserList;
	std::string XMLOut;

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		XMLOut += "<UserExperience>\n";

		EventXMLMerger merger;
		merger.LoadXML("..\\..\\Data\\Test\\XML\\GSStats.xml");
		merger.LoadXML("..\\..\\Data\\Test\\XML\\UnStats.xml");

		int CurrentUserID = -1;

		threadsafe oXML* xml;
		oXML::HNODE node;
		while(merger.GetNextEventNode(&xml, &node))
		{
			const char* name = xml->FindAttributeValue(node, "Name");
			const char* timeStamp = xml->FindAttributeValue(node, "Timestamp");

			if(0 == _stricmp("NewUser", name))
			{
				User user;
				user.StartTime = timeStamp;

				xml->GetTypedAttributeValue(node, "UserID", &user.ID);

				UserList.push_back(user);
			}
			else if(0 == _stricmp("LostUser", name))
			{
				int id = -1;
				xml->GetTypedAttributeValue(node, "UserID", &id);

				User& user = GetUser(id);
				user.EndTime = timeStamp;
				WriteXML(user);
				EraseUser(id);
			}
			else if(0 == _stricmp("NewIdealUser", name))
			{
				int id = -1;
				xml->GetTypedAttributeValue(node, "UserID", &id);

				CurrentUserID = id;
			}
			else if(0 == _stricmp("GameOver", name))
			{
				User& user = GetUser(CurrentUserID);

				user.Score = xml->FindAttributeValue(node, "Score");
				user.Spectators = (int)UserList.size() - 1;
				user.Player = true;
			}
			else if(0 == _stricmp("TakeSwordDamage", name))
			{
				User& user = GetUser(CurrentUserID);
				user.Swings++;
			}
			else if(0 == _stricmp("Hit", name))
			{
				User& user = GetUser(CurrentUserID);
				user.Hits++;
			}
		}

		XMLOut += "</UserExperience>\n";

		oFile::SaveBuffer("UserExperience.xml", XMLOut.c_str(), XMLOut.size() + 1, true, false);

		return SUCCESS;
	}

	void WriteXML(User& user)
	{
		const size_t BufSize = 256;
		char buf[BufSize];

		XMLOut += "\t<User";
		XMLOut += " StartTime=\"" + user.StartTime + "\"";
		XMLOut += " EndTime=\"" + user.EndTime + "\"";
		XMLOut += " Score=\"" + user.Score + "\"";
		sprintf_s(buf, BufSize, "%i", user.Hits);
		XMLOut += " Hits=\"" + std::string(buf) + "\"";
		sprintf_s(buf, BufSize, "%i", user.Swings);
		XMLOut += " Swings=\"" + std::string(buf) + "\"";
		sprintf_s(buf, BufSize, "%i", user.Spectators);
		XMLOut += " Spectactors=\"" + std::string(buf) + "\"";
		XMLOut += "/>\n";
	}

	User& GetUser(int UserID)
	{
		for(tUserList::iterator it = UserList.begin(); it != UserList.end(); ++it)
		{
			User& user = *it;
			if(user.ID == UserID)
			{
				return user;
			}
		}

		static User NullUser;
		return NullUser;
	}

	void EraseUser(int UserID)
	{
		for(tUserList::iterator it = UserList.begin(); it != UserList.end(); ++it)
		{
			User& user = *it;
			if(user.ID == UserID)
			{
				UserList.erase(it);
				break;
			}
		}
	}
};

oTEST_REGISTER(TESTStatParse);
