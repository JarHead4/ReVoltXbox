#ifndef _SCRIPTOBJECT_H
#define _SCRIPTOBJECT_H

#include <vector>

using namespace std;

class ScriptObject : public vector<CString>
{
	public:
		 ScriptObject(const CString& source);
	protected:
};

#endif