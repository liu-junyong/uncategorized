
#include <boost/locale/utf.hpp>
#include <boost/locale.hpp>
using namespace boost::locale::utf;



struct ParseResult {  
	code_point point;  
	size_t size;  
};  

int trail_length(char ci) {  
	unsigned char c = ci;  
	if(c < 128)  
		return 0;  
	if(BOOST_LOCALE_UNLIKELY(c < 194))  
		return -1;  
	if(c < 224)  
		return 1;  
	if(c < 240)  
		return 2;  
	if(BOOST_LOCALE_LIKELY(c <=244))  
		return 3;  
	return -1;  
}  

int width(code_point value) {  
	if(value <=0x7F) {  
		return 1;  
	}  
	else if(value <=0x7FF) {  
		return 2;  
	}  
	else if(BOOST_LOCALE_LIKELY(value <=0xFFFF)) {  
		return 3;  
	}  
	else {  
		return 4;  
	}  
}  

bool is_trail(char ci) {  
	unsigned char c = ci;  
	return (c & 0xC0) == 0x80;  
}  

bool is_lead(char ci) {  
	return !is_trail(ci);  
}  


// Convert the UTF-8 string into   
template<typename Iterator>  
void ParseUTF8(Iterator &p, Iterator e, ParseResult& result) {  
	if (BOOST_LOCALE_UNLIKELY(p == e)) {  
		//throw UnicodeError("ParseUTF8 failed");  
		return;
	}  

	unsigned char lead = *p++;  

	// First byte is fully validated here  
	int trail_size = trail_length(lead);  

	if(BOOST_LOCALE_UNLIKELY(trail_size < 0)) {  
		//throw UnicodeError("ParseUTF8 failed");  
		return;
	}  

	//  
	// Ok as only ASCII may be of size = 0  
	// also optimize for ASCII text  
	//  
	if(trail_size == 0) {  
		result.point = lead;  
		result.size = 1;  
		return;  
	}  

	code_point c = lead & ((1<<(6-trail_size))-1);  

	// Read the rest  
	unsigned char tmp;  
	switch(trail_size) {  
  case 3:  
	  if(BOOST_LOCALE_UNLIKELY(p==e)) {  
		  return;
		  //throw UnicodeError("ParseUTF8 failed");  
	  }  
	  tmp = *p++;  
	  c = (c << 6) | ( tmp & 0x3F);  
  case 2:  
	  if(BOOST_LOCALE_UNLIKELY(p==e)) {  
		  return;
		  //throw UnicodeError("ParseUTF8 failed");  
	  }  
	  tmp = *p++;  
	  c = (c << 6) | ( tmp & 0x3F);  
  case 1:  
	  if(BOOST_LOCALE_UNLIKELY(p==e)) {  
		  return;
		  //throw UnicodeError("ParseUTF8 failed");  
	  }  
	  tmp = *p++;  
	  c = (c << 6) | ( tmp & 0x3F);  
	}  

	// Check code point validity: no surrogates and  
	// valid range  
	if(BOOST_LOCALE_UNLIKELY(!is_valid_codepoint(c))) {  
		return;
		//throw UnicodeError("ParseUTF8 failed");  
	}  

	// make sure it is the most compact representation  
	if(BOOST_LOCALE_UNLIKELY(width(c) != trail_size + 1)) {  
		return;
		//throw UnicodeError("ParseUTF8 failed");  
	}  

	result.point = c;  
	result.size = trail_size + 1;  
}  


// Convert the UTF-8 string that represent one single Unicode character in [start, end) to Unicode code point  
template<typename Iterator>  
code_point UTF8ToUnicode(Iterator &start, Iterator end) {  
	ParseResult result;  
	ParseUTF8(start, end, result);  
	return result.point;  
}  

template<typename Iterator>  
code_point UTF8ToUnicode(Iterator &start, Iterator end, vector<code_point>& points) {  
	ParseResult result;  
	Iterator begin = start;  
	while (begin < end) {  
		ParseUTF8(start, end, result);  
		points.push_back(result.point);  
		begin += result.size;  
	}
	return result.point;
} 


void gbk2utf8json(string& str2)
{
	//void mbc2utf(const char *mbc, string &utf)

	string str;
	mbc2utf(str2.c_str(),str);

	string ret;
	ret.reserve(str.length()*4);
	for ( string::iterator it =  str.begin() ; it!= str.end() ;)
	{
		char tmp = *it;
		if ( tmp < 0  )
		{
			ParseResult result; 
			//string::iterator start = it;
			ParseUTF8(it, str.end(), result);  
			//it += result.size;
			char buf[8] = {0};
			sprintf(buf,"\\u%x",result.point);
			if (strlen(buf)==6)
				ret += string(buf);

		}else
		{
			//{ '&' => '\u0026', '>' => '\u003e', '<' => '\u003c', "(" => '\u2028', ")" => '\u2029' }
			if (tmp == '&')
			{
				ret += "\\u0026";
			}else if (tmp == '>')
			{
				ret += "\\u003e";
			}
			else if (tmp == '<')
			{
				ret += "\\u003c";
			}
			else if (tmp == '(')
			{
				ret += "\\u2028";
			}
			else if (tmp == ')')
			{
				ret += "\\u2029";
			}
			else if (tmp == '"')
			{
				ret += "\"";
			}
			else
			{
				char buf[2] = {0};
				buf[0] = tmp;
				ret += string(buf);
			}
			it++;
		}
	}
	str2 = ret;
}

