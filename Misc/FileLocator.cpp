/***********************************************************************
FileLocator - Class to find files from an ordered list of search paths.
Copyright (c) 2007-2025 Oliver Kreylos
Based on code written by Braden Pellett.

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Misc/FileLocator.h>

#include <string.h>
#include <stdlib.h>
#include <Misc/StdError.h>
#include <Misc/GetCurrentDirectory.h>
#include <Misc/FileTests.h>
#include <Misc/Directory.h>

namespace {

/****************
Helper functions:
****************/

const char* findLastOf(const char* string,char ch)
	{
	const char* result=0;
	for(const char* sPtr=string;*sPtr!='\0';++sPtr)
		if(*sPtr==ch)
			result=sPtr;
	return result;
	}

std::string cleanPath(const char* path)
	{
	std::string result;
	
	/* Check for the initial slash in an absolute path: */
	const char* segBegin=path;
	if(*segBegin=='/')
		{
		result.push_back('/');
		++segBegin;
		}
	
	/* Process the rest of the path segment by segment: */
	bool notFirstSegment=false;
	while(*segBegin!='\0')
		{
		/* Find the next path segment: */
		const char* segEnd;
		for(segEnd=segBegin;*segEnd!='\0'&&*segEnd!='/';++segEnd)
			;
		if(segEnd-segBegin==2&&segBegin[0]=='.'&&segBegin[1]=='.')
			{
			/* Remove the last segment from the result path: */
			while(result.size()>0&&result[result.size()-1]!='/')
				result.erase(result.size()-1);
			if(result.size()>1)
				result.erase(result.size()-1);
			}
		else if(segEnd!=segBegin&&(segEnd-segBegin!=1||*segBegin!='.'))
			{
			/* Append the segment to the result path: */
			if(notFirstSegment)
				result.push_back('/');
			result.append(segBegin,segEnd);
			notFirstSegment=true;
			}
		
		/* Go to the next path segment: */
		if(*segEnd=='/')
			++segEnd;
		segBegin=segEnd;
		}
	
	return result;
	}

std::string cleanPath(const char* pathBegin,const char* pathEnd)
	{
	std::string result;
	
	/* Check for the initial slash in an absolute path: */
	const char* segBegin=pathBegin;
	if(segBegin!=pathEnd&&*segBegin=='/')
		{
		result.push_back('/');
		++segBegin;
		}
	
	/* Process the rest of the path segment by segment: */
	bool notFirstSegment=false;
	while(segBegin!=pathEnd)
		{
		/* Find the next path segment: */
		const char* segEnd;
		for(segEnd=segBegin;segEnd!=pathEnd&&*segEnd!='/';++segEnd)
			;
		if(segEnd-segBegin==2&&segBegin[0]=='.'&&segBegin[1]=='.')
			{
			/* Remove the last segment from the result path: */
			while(result.size()>0&&result[result.size()-1]!='/')
				result.erase(result.size()-1);
			if(result.size()>1)
				result.erase(result.size()-1);
			}
		else if(segEnd!=segBegin&&(segEnd-segBegin!=1||*segBegin!='.'))
			{
			/* Append the segment to the result path: */
			if(notFirstSegment)
				result.push_back('/');
			result.append(segBegin,segEnd);
			notFirstSegment=true;
			}
		
		/* Go to the next path segment: */
		if(segEnd!=pathEnd)
			++segEnd;
		segBegin=segEnd;
		}
	
	return result;
	}

}

namespace Misc {

/*****************************************
Methods of class FileLocator:FileNotFound:
*****************************************/

FileLocator::FileNotFound::FileNotFound(const char* source,const char* sFileName)
	:std::runtime_error(makeStdErrMsg(source,"File \"%s\" not found",sFileName).c_str()),
	 fileName(sFileName)
	{
	}

/****************************
Methods of class FileLocator:
****************************/

FileLocator::FileLocator(void)
	{
	}

void FileLocator::addCurrentDirectory(void)
	{
	pathList.push_back(getCurrentDirectory());
	}

void FileLocator::addPath(const char* newPath)
	{
	/* Check if the path is fully-qualified: */
	if(newPath[0]=='/')
		pathList.push_back(cleanPath(newPath));
	else if(newPath[0]!='\0')
		{
		/* Prefix relative paths with the current working directory: */
		std::string absPath=getCurrentDirectory();
		absPath.push_back('/');
		absPath.append(newPath);
		pathList.push_back(cleanPath(absPath.c_str()));
		}
	}

void FileLocator::addPath(const char* newPathBegin,const char* newPathEnd)
	{
	/* Check if the path is fully-qualified: */
	if(newPathEnd-newPathBegin>1&&newPathBegin[0]=='/')
		pathList.push_back(cleanPath(newPathBegin,newPathEnd));
	else if(newPathEnd-newPathBegin>0)
		{
		/* Prefix relative paths with the current working directory: */
		std::string absPath=getCurrentDirectory();
		absPath.push_back('/');
		absPath.append(newPathBegin,newPathEnd);
		pathList.push_back(cleanPath(absPath.c_str()));
		}
	}

void FileLocator::addPathFromFile(const char* fileName)
	{
	/* Find the final '/': */
	const char* slashPtr=findLastOf(fileName,'/');
	if(slashPtr!=0)
		{
		/* Add the file's directory to the search path list: */
		addPath(fileName,slashPtr);
		}
	else
		{
		/* Add the current directory to the search path list: */
		pathList.push_back(getCurrentDirectory());
		}
	}

void FileLocator::addPathList(const char* newPathList)
	{
	/* Process all paths from the colon-separated list: */
	while(*newPathList!='\0')
		{
		/* Find the end of the next path: */
		const char* pathEnd;
		for(pathEnd=newPathList;*pathEnd!='\0'&&*pathEnd!=':';++pathEnd)
			;
		
		/* Add the path if it is non-empty: */
		if(pathEnd!=newPathList)
			addPath(newPathList,pathEnd);
		
		/* Go to the next path: */
		if(*pathEnd!='\0')
			++pathEnd;
		newPathList=pathEnd;
		}
	}

void FileLocator::addPathFromApplication(const char* executablePath)
	{
	/* Separate the executable name from the executable path: */
	const char* slashPtr=findLastOf(executablePath,'/');
	std::string appName=slashPtr!=0?std::string(slashPtr+1):std::string(executablePath);
	
	/* Add the standard resource search path for private installed applications: */
	if(getenv("HOME")!=0)
		{
		std::string path=getenv("HOME");
		path.append("/.");
		path.append(appName);
		addPath(path);
		}
	
	/* Add the standard resource search paths for system-wide installed applications: */
	addPath("/usr/share/"+appName);
	addPath("/usr/local/share/"+appName);
	
	/* Construct the fully-qualified executable name: */
	std::string fullExePath;
	if(executablePath[0]=='/')
		fullExePath=executablePath;
	else
		{
		/* Try to find the full path to the executable: */
		#if 0
		/* Get the full executable path through the /proc interface: */
		char pse[PATH_MAX];
		int pseLength=readlink("/proc/self/exe",pse,PATH_MAX);
		if(pseLength>=0)
			fullExePath=std::string(pse,pse+pseLength);
		else
			{
		#else
		/* Search for the executable just as the shell did: */
		if(slashPtr==0&&getenv("PATH")!=0)
			{
			/* Search for the executable in the PATH list: */
			const char* pathBegin=getenv("PATH");
			while(*pathBegin!='\0')
				{
				/* Find the end of the next path: */
				const char* pathEnd;
				for(pathEnd=pathBegin;*pathEnd!='\0'&&*pathEnd!=':';++pathEnd)
					;
				
				/* Test the path if it is non-empty: */
				if(pathEnd!=pathBegin)
					{
					/* Construct the full path name: */
					std::string testName;
					if(*pathBegin!='/')
						{
						/* Start the path name with the current directory: */
						testName=getCurrentDirectory();
						testName.push_back('/');
						}
					testName.append(pathBegin,pathEnd);
					testName.push_back('/');
					testName.append(appName);
					
					/* Test if the file exists and is an executable: */
					if(Misc::isPathFile(testName.c_str()))
						{
						/* Save the matching full path and stop searching: */
						fullExePath=testName;
						break;
						}
					}
				
				/* Go to the next path: */
				if(*pathEnd!='\0')
					++pathEnd;
				pathBegin=pathEnd;
				}
			}
		else
			{
			/* Use the provided path starting at the current directory: */
			fullExePath=getCurrentDirectory();
			fullExePath.push_back('/');
			fullExePath.append(executablePath);
			}
		#endif
		#if 0
			}
		#endif
		}
	
	/* Find the last slash in the cleaned fully-qualified executable path name: */
	std::string cleanFullExePath=cleanPath(fullExePath.c_str());
	executablePath=cleanFullExePath.c_str();
	slashPtr=findLastOf(executablePath,'/');
	
	#ifdef __linux__
	/* Check if the executable is part of a Linux application bundle: */
	if(slashPtr!=0)
		{
		if(slashPtr-executablePath>=4&&strncasecmp(slashPtr-4,"/exe",4)==0)
			{
			/* Add the bundle's base directory to the search path list: */
			addPath(executablePath,slashPtr-4);
			}
		else if(slashPtr-executablePath>=7&&strncasecmp(slashPtr-7,"/exe/64",7)==0)
			{
			/* Add the bundle's base directory to the search path list: */
			addPath(executablePath,slashPtr-7);
			}
		}
	#endif
	
	#ifdef __APPLE__
	/* Check if the executable is part of a Mac OS X application bundle: */
	if(slashPtr!=0&&slashPtr-executablePath>=19&&strncasecmp(slashPtr-19,".app/Contents/MacOS",19)==0)
		{
		/* Add the bundle's resource directory to the search path list: */
		addPath(std::string(executablePath,slashPtr-5)+"Resources");
		}
	#endif
	}

std::string FileLocator::locateFile(const char* fileName) const
	{
	/* Strip a potential path prefix of the given file name first: */
	const char* actualFileName=fileName;
	for(const char* fnPtr=fileName;*fnPtr!='\0';++fnPtr)
		if(*fnPtr=='/')
			actualFileName=fnPtr+1;
	
	/* If the given file name has a path prefix, check the given path first: */
	if(actualFileName!=fileName&&Misc::doesPathExist(fileName))
		return fileName;
	
	/* Look for a file of the actual given name in all search paths in the list: */
	for(std::vector<std::string>::const_iterator plIt=pathList.begin();plIt!=pathList.end();++plIt)
		{
		/* Check if a file of the actual given name exists in the search path: */
		std::string pathName=*plIt;
		pathName.push_back('/');
		pathName+=actualFileName;
		if(Misc::doesPathExist(pathName.c_str()))
			return pathName;
		}
	
	/* Instead of returning an empty string or somesuch, throw an exception: */
	throw FileNotFound(__PRETTY_FUNCTION__,actualFileName);
	}

std::string FileLocator::locateNumberedFile(const char* fileNameTemplate) const
	{
	/* Find the file name template's potential directory prefix and conversion location: */
	const char* fileNameStart=fileNameTemplate;
	const char* conversion=0;
	const char* fntPtr;
	for(fntPtr=fileNameTemplate;*fntPtr!='\0';++fntPtr)
		{
		if(*fntPtr=='/')
			fileNameStart=fntPtr+1;
		else if(*fntPtr=='%'&&fntPtr[1]!='%')
			{
			if(fntPtr[1]=='u')
				{
				if(conversion!=0)
					throw makeStdErr(__PRETTY_FUNCTION__,"Template \"%s\" contains more than one %%u conversion",fileNameTemplate);
				conversion=fntPtr;
				}
			else if(fntPtr[1]!='\0')
				throw makeStdErr(__PRETTY_FUNCTION__,"Template \"%s\" contains invalid %%%c conversion",fileNameTemplate,fntPtr[1]);
			else
				throw makeStdErr(__PRETTY_FUNCTION__,"Template \"%s\" contains dangling %%",fileNameTemplate);
			}
		}
	if(conversion==0)
		throw makeStdErr(__PRETTY_FUNCTION__,"Template \"%s\" does not contain a %%u conversion",fileNameTemplate);
	if(conversion<fileNameStart)
		throw makeStdErr(__PRETTY_FUNCTION__,"Template \"%s\" has %%u conversion in directory prefix",fileNameTemplate);
	size_t preConversionLen=conversion-fileNameStart;
	
	/* Find the highest-numbered file that matches the given template in all search paths: */
	std::string result;
	unsigned int highestNumber=(unsigned int)(-1);
	for(std::vector<std::string>::const_iterator plIt=pathList.begin();plIt!=pathList.end();++plIt)
		{
		try
			{
			/* Open the search path as a directory: */
			std::string dirName=*plIt;
			if(fileNameStart!=fileNameTemplate)
				dirName.append(fileNameTemplate,fileNameStart);
			Directory dir(dirName.c_str());
			
			/* Search for files matching the given template: */
			while(!dir.eod())
				{
				/* Match the entry name: */
				const char* enPtr=dir.getEntryName();
				if(strncmp(enPtr,fileNameStart,preConversionLen)==0)
					{
					/* Convert a sequence of digits to a number: */
					enPtr+=preConversionLen;
					unsigned int number=0;
					while(*enPtr>='0'&&*enPtr<='9')
						{
						number=number*10U+(unsigned int)(*enPtr-'0');
						++enPtr;
						}
					
					/* Compare the entry name's suffix: */
					if(strcmp(enPtr,conversion+2)==0)
						{
						/* Check if this match has the highest number: */
						if(highestNumber==(unsigned int)(-1)||highestNumber<number)
							{
							result=dirName;
							result.push_back('/');
							result.append(dir.getEntryName());
							highestNumber=number;
							}
						}
					}
				
				/* Read the next directory entry: */
				dir.readNextEntry();
				}
			}
		catch(const std::runtime_error&)
			{
			/* Ignore the error and carry on... */
			}
		}
	
	if(highestNumber==(unsigned int)(-1))
		throw FileNotFound(__PRETTY_FUNCTION__,fileNameTemplate);
	
	return result;
	}

}
