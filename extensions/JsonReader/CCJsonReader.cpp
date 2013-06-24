/*
* Copyright (c) 2012 Chukong Technologies, Inc.
*
* http://www.cocostudio.com
* http://tools.cocoachina.com
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
* NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
* USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "CCJsonReader.h"
#include "cocos-ext.h"
#include "DictionaryHelper.h"
#include "CCArmature.h"
#include "CCArmatureDataManager.h"

NS_CC_EXT_BEGIN

	CCJsonReader* CCJsonReader::s_sharedJsonReader = NULL;

CCJsonReader::CCJsonReader()
{
}

CCJsonReader::~CCJsonReader()
{
}

cocos2d::CCNode* CCJsonReader::createNodeWithJsonFile(const char* pszFileName)
{
	unsigned long size = 0;
	const char* pData = 0;

	do {
		pData = (char*)(cocos2d::CCFileUtils::sharedFileUtils()->getFileData(pszFileName, "r", &size));
		CC_BREAK_IF(pData == NULL || strcmp(pData, "") == 0);
		cs::CSJsonDictionary *jsonDict = new cs::CSJsonDictionary();
		jsonDict->initWithDescription(pData);
		CCNode* gb = createObject(jsonDict,NULL);
		CC_SAFE_DELETE(jsonDict);
		return gb;
	} while (0);

	return NULL;
}

CCNode* CCJsonReader::createObject(cs::CSJsonDictionary * inputFiles, CCNode* parenet)
{
	const char* className = inputFiles->getItemStringValue("classname"); 

	if(strcmp(className, "CCNode") == 0)
	{
		CCNode* gb = NULL;
		if(NULL == parenet)
		{
			gb = CCNode::create();
		}
		else
		{
			gb = CCNode::create();
			parenet->addChild(gb);
		}

		setPropertyFromJsonDict(gb, inputFiles);

		int count = inputFiles->getArrayItemCount("components");
		for (int i = 0; i < count; i++)
		{
			cs::CSJsonDictionary * subDict = inputFiles->getSubItemFromArray("components", i);
			if (!subDict)
				break;
			const char* comName = subDict->getItemStringValue("classname");
			const char *file = subDict->getItemStringValue("file");
			const char *name = subDict->getItemStringValue("name");
			if (file == NULL)
			{
				continue;
			}
			CCAssert(file != NULL, "file must be not NULL!");
			std::string pPath = cocos2d::CCFileUtils::sharedFileUtils()->fullPathForFilename(file);

			if (strcmp(comName, "CCSprite") == 0)
			{
				cocos2d::CCSprite *pSprite = CCSprite::create(pPath.c_str());
				CCComRender *pRender = CCComRender::create(pSprite, "CCSprite");
				if (name != NULL)
				{
					pRender->setName(name);
				}
				gb->addComponent(pRender);
			}
			else if(strcmp(comName, "CCTMXTiledMap") == 0)
			{
				cocos2d::CCTMXTiledMap *pTmx = CCTMXTiledMap::create(pPath.c_str());
				CCComRender *pRender = CCComRender::create(pTmx, "CCTMXTiledMap");
				if (name != NULL)
				{
					pRender->setName(name);
				}
				gb->addComponent(pRender);
			}
			else if(strcmp(comName, "CCParticleSystemQuad") == 0)
			{
				cocos2d::CCParticleSystemQuad *pParticle = CCParticleSystemQuad::create(pPath.c_str());
				CCComRender *pRender = CCComRender::create(pParticle, "CCParticleSystemQuad");
				if (name != NULL)
				{
					pRender->setName(name);
				}
				gb->addComponent(pRender);
			}
			else if(strcmp(comName, "CCArmature") == 0)
			{
				std::string reDir = pPath;
				std::string file_path = "";
				size_t pos = reDir.find_last_of('/');
				if (pos != std::string::npos)
				{
					file_path = reDir.substr(0, pos+1);
				}
				unsigned long size = 0;
				const char *des = (char*)(cocos2d::CCFileUtils::sharedFileUtils()->getFileData(pPath.c_str(),"r" , &size));
				cs::CSJsonDictionary *jsonDict = new cs::CSJsonDictionary();
				jsonDict->initWithDescription(des);
				if(NULL == des || strcmp(des, "") == 0)
				{
					CCLog("read json file[%s] error!\n", pPath.c_str());
				}
				int childrenCount = DICTOOL->getArrayCount_json(jsonDict, "armature_data");
				cs::CSJsonDictionary* subData = DICTOOL->getDictionaryFromArray_json(jsonDict, "armature_data", 0);
				const char *name = DICTOOL->getStringValue_json(subData, "name");

				childrenCount = DICTOOL->getArrayCount_json(jsonDict, "config_file_path");
				for (int i = 0; i < childrenCount; ++i)
				{
					const char* plist = DICTOOL->getStringValueFromArray_json(jsonDict, "config_file_path", 0);
					std::string plistpath;
					plistpath += file_path;
					plistpath.append(plist);
					cocos2d::CCDictionary *root = CCDictionary::createWithContentsOfFile(plistpath.c_str());
					CCDictionary* metadata = DICTOOL->getSubDictionary(root, "metadata");
					const char* textureFileName = DICTOOL->getStringValue(metadata, "textureFileName");

					std::string textupath;
					textupath += file_path;
					textupath.append(textureFileName);
					CCArmatureDataManager::sharedArmatureDataManager()->addArmatureFileInfo(textupath.c_str(), plistpath.c_str(), pPath.c_str());

				}

				CCArmature *pAr = CCArmature::create(name);
				CCComRender *pRender = CCComRender::create(pAr, "CCArmature");
				if (name != NULL)
				{
					pRender->setName(name);
				}
				gb->addComponent(pRender);

				CC_SAFE_DELETE(jsonDict);
			}
			else if(strcmp(comName, "CCComAudio") == 0)
			{
				CCComAudio *pAudio = CCComAudio::create();
				pAudio->preloadEffect(pPath.c_str());
				if (name != NULL)
				{
					pAudio->setName(name);
				}
				gb->addComponent(pAudio);
			}
			else if(strcmp(comName, "CCComAttribute") == 0)
			{
				CCComAttribute *pAttribute = CCComAttribute::create();
				if (name != NULL)
				{
					pAttribute->setName(name);
				}
				gb->addComponent(pAttribute);
			}
			else if (strcmp(comName, "CCBackgroundAudio") == 0)
			{
				CCComAudio *pAudio = CCComAudio::create();
				pAudio->preloadBackgroundMusic(pPath.c_str());
				if (name != NULL)
				{
					pAudio->setName(name);
				}
				gb->addComponent(pAudio);
			}
		}

		for (int i = 0; i < inputFiles->getArrayItemCount("gameobjects"); i++)
		{
			cs::CSJsonDictionary * subDict = inputFiles->getSubItemFromArray("gameobjects", i);
			if (!subDict)
			{
				break;
			}
			createObject(subDict, gb);
		}

		return gb;
	}

	return NULL;
}


void CCJsonReader::setPropertyFromJsonDict(cocos2d::CCNode *node, cs::CSJsonDictionary* dict)
{
	int x = dict->getItemIntValue("x", 0);
	int y = dict->getItemIntValue("y", 0);
	node->setPosition(ccp(x, y));

	bool bVisible = (bool)(dict->getItemIntValue("visible", 1));
	node->setVisible(bVisible);

	int nTag = dict->getItemIntValue("objecttag", -1);
	node->setTag(nTag);

	int nZorder = dict->getItemIntValue("zorder", 0);
	node->setZOrder(nZorder);

	float fScaleX = dict->getItemFloatValue("scalex", 1.0);
	float fScaleY = dict->getItemFloatValue("scaley", 1.0);
	node->setScaleX(fScaleX);
	node->setScaleY(fScaleY);

	float fRotationZ = dict->getItemIntValue("rotation", 0);
	node->setRotation(fRotationZ);
}

CCJsonReader* CCJsonReader::sharedJsonReader()
{
	if (s_sharedJsonReader == NULL)
	{
		s_sharedJsonReader = new CCJsonReader();
	}
	return s_sharedJsonReader;
}

void CCJsonReader::purgeJsonReader()
{
	CC_SAFE_DELETE(s_sharedJsonReader);
}

NS_CC_EXT_END
