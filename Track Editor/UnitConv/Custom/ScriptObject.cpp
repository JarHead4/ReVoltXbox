#include "stdafx.h"
#include "ScriptObject.h"


ScriptObject::ScriptObject(const CString& source)
{
	CString scriptstring(source);
	
	scriptstring.TrimLeft(); //remove any leading whitespace

	CString line;
	while( !scriptstring.IsEmpty() )
	{
		int pos = scriptstring.FindOneOf("\n\r");	//search for line delimiters
		if(pos == -1)	//if not found
		{
			line = scriptstring;	//just copy the entire string
			scriptstring.Empty();	//and clear out the string
		}
		else
		{
			line = scriptstring.Left(pos);	//copy the characters before delimiter
			scriptstring = scriptstring.Mid(pos); //remove the section we just copied
			scriptstring.TrimLeft(); //remove any leading whitespace
		}
		push_back(line);
	}
}

