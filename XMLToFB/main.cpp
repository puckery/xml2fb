/*
*  xml to flatbuffers
*  author: gugu
*/

#include <string>
using namespace std;

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "tinyxml/tinyxml2.h"
using namespace flatbuffers;

#include "dragonbones_generated.h"
#include "dragonbones_texture_generated.h"

const std::string FILE_EXT = ".bin";

flatbuffers::FlatBufferBuilder* _builder;

std::string getAttribute(const tinyxml2::XMLElement* element, const std::string& name, const std::string& defaultValue = "")
{
	if (element->FindAttribute(name.c_str()))
	{
		return element->Attribute(name.c_str());
	}
	return defaultValue;
}

int getIntAttribute(const tinyxml2::XMLElement* element, const std::string& name, int defaultValue = 0)
{
	if (element->FindAttribute(name.c_str()))
	{
		return element->IntAttribute(name.c_str());
	}
	return defaultValue;
}

float getFloatAttribute(const tinyxml2::XMLElement* element, const std::string& name, float defaultValue = 0.f)
{
	if (element->FindAttribute(name.c_str()))
	{
		return element->FloatAttribute(name.c_str());
	}
	return defaultValue;
}

bool getBoolAttribute(const tinyxml2::XMLElement* element, const std::string& name, bool defaultValue = false)
{
	if (element->FindAttribute(name.c_str()))
	{
		return element->BoolAttribute(name.c_str());
	}
	return defaultValue;
}

Transform createTransformStruct(const tinyxml2::XMLElement* element)
{
	float x =	getFloatAttribute(element, "x", 0);
	float y =	getFloatAttribute(element, "y", 0);
	float skX = getFloatAttribute(element, "skX", 0);
	float skY = getFloatAttribute(element, "skY", 0);
	float scX = getFloatAttribute(element, "scX", 1.0f);
	float scY = getFloatAttribute(element, "scY", 1.0f);

	return Transform(x, y, skX, skY, scX, scY, 0.0f, 0.0f);
}

Offset<Bone> createBoneTable(const tinyxml2::XMLElement* element)
{
	std::string name = getAttribute(element, "name");
	std::string parent = getAttribute(element, "parent", "");
	int length = getIntAttribute(element, "length", 0);

	// create transform
	const tinyxml2::XMLElement* child = element->FirstChildElement();
	assert(child);
	Transform transformStruct = createTransformStruct(child);
	
	return CreateBone(
		*_builder,
		_builder->CreateString(name),
		_builder->CreateString(parent),
		length,
		&transformStruct);
}

Offset<Skin> createSkinTable(const tinyxml2::XMLElement* element)
{
	std::string name = getAttribute(element, "name");

	Offset<Slot> slotTable;
	std::vector<Offset<Slot>> slots;

	const tinyxml2::XMLElement* slotChild = element->FirstChildElement();
	while (slotChild)
	{
		std::string slotName = getAttribute(slotChild, "name");
		std::string parent = getAttribute(slotChild, "parent");
		float z = getFloatAttribute(slotChild, "z", 0.0f);
		std::string blendMode = getAttribute(slotChild, "blendMode");

		// displays
		Offset<Display> displayTable;
		std::vector<Offset<Display>> displays;

		const tinyxml2::XMLElement* displayChild = slotChild->FirstChildElement();
		while (displayChild)
		{
			std::string displayType = getAttribute(displayChild, "type");
			std::string displayName = getAttribute(displayChild, "name");

			// create transform
			const tinyxml2::XMLElement* transformChild = displayChild->FirstChildElement();
			assert(transformChild);
			
			float x = getFloatAttribute(transformChild, "x", 0.0f);
			float y = getFloatAttribute(transformChild, "y", 0.0f);
			float skX = getFloatAttribute(transformChild, "skX", 0.0f);
			float skY = getFloatAttribute(transformChild, "skY", 0.0f);
			float scX = getFloatAttribute(transformChild, "scX", 1.0f);
			float scY = getFloatAttribute(transformChild, "scY", 1.0f);
			float pX = getFloatAttribute(transformChild, "pX", 0.0f);
			float pY = getFloatAttribute(transformChild, "pY", 0.0f);
			Transform transform(x, y, skX, skY, scX, scY, pX, pY);

			auto display = CreateDisplay(
				*_builder,
				_builder->CreateString(displayType),
				_builder->CreateString(displayName),
				&transform);
			displays.push_back(display);

			// next display
			displayChild = displayChild->NextSiblingElement();
		}

		// create slot
		auto slot = CreateSlot(
			*_builder,
			_builder->CreateString(slotName),
			_builder->CreateString(parent),
			_builder->CreateString(blendMode),
			z,
			_builder->CreateVector(displays));
		slots.push_back(slot);
		
		// next slot
		slotChild = slotChild->NextSiblingElement();
	}

	return CreateSkin(
		*_builder,
		_builder->CreateString(name),
		_builder->CreateVector(slots));
}

Offset<Frame> createFrameTable(const tinyxml2::XMLElement* element)
{
	int duration = getIntAttribute(element, "duration");
	std::string event = getAttribute(element, "event");
	std::string sound = getAttribute(element, "sound");
	std::string action = getAttribute(element, "action");

	return CreateFrame(
		*_builder,
		_builder->CreateString(event),
		_builder->CreateString(sound),
		_builder->CreateString(action),
		duration);
}

ColorTransform createColorTransformStruct(const tinyxml2::XMLElement* element)
{
	int aO = getIntAttribute(element, "aO", 0);
	int rO = getIntAttribute(element, "rO", 0);
	int gO = getIntAttribute(element, "gO", 0);
	int bO = getIntAttribute(element, "bO", 0);
	int aM = getIntAttribute(element, "aM", 100);
	int rM = getIntAttribute(element, "rM", 100);
	int gM = getIntAttribute(element, "gM", 100);
	int bM = getIntAttribute(element, "bM", 100);

	return ColorTransform(aO, rO, gO, bO, aM, rM, gM, bM);
}

Offset<Timeline_Frame> createTimeline_FrameTable(const tinyxml2::XMLElement* element)
{
	int duration = getIntAttribute(element, "duration");
	std::string event = getAttribute(element, "event");
	std::string sound = getAttribute(element, "sound");
	std::string action = getAttribute(element, "action");
	bool hide = getBoolAttribute(element, "hide", 0);
	std::string tweenEasing = getAttribute(element, "tweenEasing", "0");
	int tweenRotate = getIntAttribute(element, "tweenRotate", 0);
	int displayIndex = getIntAttribute(element, "displayIndex", 0);
	float z = getFloatAttribute(element, "z", 0.f);

	// create transform
	Transform transformStruct(0, 0, 0, 0, 0, 0, 0, 0);
	ColorTransform colorTransformStruct(0, 0, 0, 0, 100, 100, 100, 100);

	Offset<Timeline_Frame> timeline_FrameTable;

	const tinyxml2::XMLElement* childElement = element->FirstChildElement();
	while (childElement)
	{
		std::string name = childElement->Name();
		if (name == "transform")
		{
			transformStruct = createTransformStruct(childElement);
		}
		else if (name == "colorTransform")
		{
			colorTransformStruct = createColorTransformStruct(childElement);
		}
		childElement = childElement->NextSiblingElement();
	}

	return CreateTimeline_Frame(
		*_builder,
		_builder->CreateString(event),
		_builder->CreateString(sound),
		_builder->CreateString(action),
		_builder->CreateString(tweenEasing),
		z,
		tweenRotate,
		duration,
		displayIndex,
		hide,
		&transformStruct,
		&colorTransformStruct);
}

Offset<Timeline> createTimelineTable(const tinyxml2::XMLElement* element)
{
	std::string name = getAttribute(element, "name");
	float scale = getFloatAttribute(element, "scale", 1.f);
	float offset = getFloatAttribute(element, "offset", 0.f);
	float pX = getFloatAttribute(element, "pX", 0.f);
	float pY = getFloatAttribute(element, "pY", 0.f);
	
	// create timeline frame
	Offset<Timeline_Frame> timeline_frameTable;
	std::vector<Offset<Timeline_Frame>> timeline_frames;

	const tinyxml2::XMLElement* childElement = element->FirstChildElement();
	while (childElement)
	{
		timeline_frameTable = createTimeline_FrameTable(childElement);
		timeline_frames.push_back(timeline_frameTable);

		childElement = childElement->NextSiblingElement();
	}

	return CreateTimeline(
		*_builder,
		_builder->CreateString(name),
		scale,
		offset,
		pX,
		pY,
		_builder->CreateVector(timeline_frames));
}

Offset<Animation> createAnimationTable(const tinyxml2::XMLElement* element)
{
	std::string name = getAttribute(element, "name");
	float fadeInTime = getFloatAttribute(element, "fadeInTime", 0.0f);
	int duration = getIntAttribute(element, "duration");
	float scale = getFloatAttribute(element, "scale", 1.0f);
	int loop = getIntAttribute(element, "loop", 1);
	std::string tweenEasing = getAttribute(element, "tweenEasing", "NaN");
	int autoTween = getIntAttribute(element, "autoTween", 1);

	// main frames
	Offset<Frame> frameTable;
	std::vector<Offset<Frame>> frames;

	Offset<Timeline> timelineTable;
	std::vector<Offset<Timeline>> timelines;

	const tinyxml2::XMLElement* childElement = element->FirstChildElement();
	while (childElement)
	{
		std::string childName = childElement->Name();
		if (childName == "frame")
		{
			frameTable = createFrameTable(childElement);
			frames.push_back(frameTable);
		}
		else if (childName == "timeline")
		{
			timelineTable = createTimelineTable(childElement);
			timelines.push_back(timelineTable);
		}
		childElement = childElement->NextSiblingElement();
	}

	return CreateAnimation(
		*_builder,
		_builder->CreateString(name),
		_builder->CreateString(tweenEasing),
		fadeInTime,
		scale,
		loop,
		duration,
		autoTween,
		_builder->CreateVector(frames),
		_builder->CreateVector(timelines));
}

Offset<Armature> createArmatureTable(const tinyxml2::XMLElement* element)
{
	// create armature table
	std::string armatureName = element->Attribute("name");

	// create bone skin animation
	Offset<Bone> boneTable;
	std::vector<Offset<Bone>> bones;
	Offset<Skin> skinTable;
	std::vector<Offset<Skin>> skins;
	Offset<Animation> animationTable;
	std::vector<Offset<Animation>> animtions;

	const tinyxml2::XMLElement* armatureChildElement = element->FirstChildElement();
	while (armatureChildElement)
	{
		std::string name = armatureChildElement->Name();
		if (name == "bone")
		{
			// bone list
			const tinyxml2::XMLElement* childElement = armatureChildElement;
			boneTable = createBoneTable(childElement);
			bones.push_back(boneTable);
		}
		else if (name == "skin")
		{
			// skin list
			const tinyxml2::XMLElement* childElement = armatureChildElement;
			skinTable = createSkinTable(childElement);
			skins.push_back(skinTable);
		}
		else if (name == "animation")
		{
			// animation list
			const tinyxml2::XMLElement* childElement = armatureChildElement;
			animationTable = createAnimationTable(childElement);
			animtions.push_back(animationTable);
		}
			armatureChildElement = armatureChildElement->NextSiblingElement();
	}

	// create armature 
	return CreateArmature(
		*_builder,
		_builder->CreateString(armatureName),
		_builder->CreateVector(bones),
		_builder->CreateVector(animtions),
		_builder->CreateVector(skins));
}

Offset<Dragonbones> createDragonbonesTable(const tinyxml2::XMLElement* element)
{
	// get infos
	std::string dragonBonesName = getAttribute(element, "name");
	int frameRate = getIntAttribute(element, "frameRate", 30);
	std::string version = getAttribute(element, "version");
	bool isGlobal = getBoolAttribute(element, "isGlobal", true);

	// create armatures
	std::vector<Offset<Armature>> armatures;
	Offset<Armature> armature;
	const tinyxml2::XMLElement* childElement = element->FirstChildElement();
	while (childElement)
	{
		armature = createArmatureTable(childElement);
		armatures.push_back(armature);
		childElement = childElement->NextSiblingElement();
	}

	// create dragonbones table
	Offset<Dragonbones> dragonBonesTable;
	return CreateDragonbones(*_builder, 
		_builder->CreateString(dragonBonesName),
		_builder->CreateString(version),
		frameRate,
		isGlobal,
		_builder->CreateVector(armatures));
}

// -------------------------------- texture atlas begin --------------------------------------//
Offset<SubTexture> createSubTextureTable(const tinyxml2::XMLElement* element)
{
	std::string name = getAttribute(element, "name");

	float x = getFloatAttribute(element, "x");
	float y = getFloatAttribute(element, "y");
	float width = getFloatAttribute(element, "width");
	float height = getFloatAttribute(element, "height");
	float frameWidth = getFloatAttribute(element, "frameWidth");
	float frameHeight = getFloatAttribute(element, "frameHeight");
	float frameX = getFloatAttribute(element, "frameX");
	float frameY = getFloatAttribute(element, "frameY");
	bool rotated = getBoolAttribute(element, "rotated");

	return CreateSubTexture(
		*_builder,
		_builder->CreateString(name),
		x,
		y,
		width,
		height,
		frameWidth,
		frameHeight,
		frameX,
		frameY,
		rotated);
}

Offset<TextureAtlas> createTextureAtlasTable(const tinyxml2::XMLElement* element)
{
	std::string name = getAttribute(element, "name");
	std::string imagePath = getAttribute(element, "imagePath");

	// create subtextures
	Offset<SubTexture> subTextureTable;
	std::vector<Offset<SubTexture>> subTextures;
	const tinyxml2::XMLElement* childElement = element->FirstChildElement();
	while (childElement)
	{
		subTextureTable = createSubTextureTable(childElement);
		subTextures.push_back(subTextureTable);
		childElement = childElement->NextSiblingElement();
	}

	return CreateTextureAtlas(
		*_builder,
		_builder->CreateString(name),
		_builder->CreateString(imagePath),
		_builder->CreateVector(subTextures));
}
// -------------------------------- texture atlas end --------------------------------------//

enum BUILD_TYPE
{
	SKELETON,
	TEXTURE
};

bool buildBinary(BUILD_TYPE buildType, const std::string& orignFilePath, const std::string& savePath)
{
	std::string fileData;
	bool ok = flatbuffers::LoadFile(orignFilePath.c_str(), true, &fileData);
	if (!ok)
	{
		return false;
	}

	_builder = new flatbuffers::FlatBufferBuilder();

	tinyxml2::XMLDocument* document = new tinyxml2::XMLDocument();
	document->Parse(fileData.c_str());

	// get dragonBones info
	const tinyxml2::XMLElement* rootElement = document->RootElement();

	switch (buildType)
	{
	case SKELETON:
		{
			auto binary = createDragonbonesTable(rootElement);
			_builder->Finish(binary);
		}
		break;
	case TEXTURE:
		{
			auto binary = createTextureAtlasTable(rootElement);
			_builder->Finish(binary);
		}
		break;
	default:
		printf("error build type");
		break;
	}

	// save binary file
	auto save = flatbuffers::SaveFile(savePath.c_str(),
		reinterpret_cast<const char*>(_builder->GetBufferPointer()),
		_builder->GetSize(),
		true);
	
	_builder->Clear();
	delete _builder;
	_builder = nullptr;

	if (!save) return false;

	return true;
}

std::string getSavePath(const std::string& filePath, const std::string& ext)
{
	auto dirPath = flatbuffers::StripFileName(filePath);
	auto baseName = flatbuffers::StripPath(flatbuffers::StripExtension(filePath));

	dirPath.append("\\");
	dirPath.append(baseName);
	dirPath.append(ext);

	return dirPath;
}



int main(int argc, const char * argv[]) 
{
	// 判断要转换的文件
	bool isSucc = false;
	for (int argi = 1; argi < argc; argi++)
	{
		const char *arg = argv[argi];
		if (arg[0] == '-')
		{
			std::string opt = arg;
			if (opt == "-s")
			{
				// skeleton.xml
				std::string skeletonFile = argv[argi + 1];
				printf("%s", skeletonFile.c_str());
				auto savePath = getSavePath(skeletonFile, FILE_EXT);

				isSucc = buildBinary(SKELETON, skeletonFile, savePath);
			}
			else if (opt == "-t")
			{
				// texture.xml
				std::string textureFile = argv[argi + 1];
				printf("%s", textureFile.c_str());
				auto savePath = getSavePath(textureFile, FILE_EXT);

				isSucc = buildBinary(TEXTURE, textureFile, savePath);
			}
			else
			{
				printf("error opt args");
			}
		}
	}
	return 1;
}
