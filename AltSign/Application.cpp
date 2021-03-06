//
//  Application.cpp
//  AltSign-Windows
//
//  Created by Riley Testut on 8/12/19.
//  Copyright © 2019 Riley Testut. All rights reserved.
//

#include "Application.hpp"

#include "Error.hpp"

#include <fstream>
#include <filesystem>

extern std::vector<unsigned char> readFile(const char* filename);

namespace fs = std::filesystem;

Application::Application()
{
}

Application::~Application()
{
}

Application::Application(std::string appBundlePath)
{
    fs::path path(appBundlePath);
    path.append("Info.plist");

	auto plistData = readFile(path.string().c_str());

    plist_t plist = nullptr;
    plist_from_memory((const char *)plistData.data(), (int)plistData.size(), &plist);
    if (plist == nullptr)
    {
        throw SignError(SignErrorCode::InvalidApp);
    }
    
    auto nameNode = plist_dict_get_item(plist, "CFBundleName");
    auto bundleIdentifierNode = plist_dict_get_item(plist, "CFBundleIdentifier");
    auto versionNode = plist_dict_get_item(plist, "CFBundleShortVersionString");

    if (nameNode == nullptr || bundleIdentifierNode == nullptr || versionNode == nullptr)
    {
        throw SignError(SignErrorCode::InvalidApp);
    }
    
    char *name = nullptr;
    plist_get_string_val(nameNode, &name);

    char *bundleIdentifier = nullptr;
    plist_get_string_val(bundleIdentifierNode, &bundleIdentifier);
    
    char *version = nullptr;
    plist_get_string_val(versionNode, &version);

    _name = name;
    _bundleIdentifier = bundleIdentifier;
    _version = version;
    _path = appBundlePath;
}


#pragma mark - Description -

std::ostream& operator<<(std::ostream& os, const Application& app)
{
    os << "Name: " << app.name() << " ID: " << app.bundleIdentifier();
    return os;
}
    
#pragma mark - Getters -
    
std::string Application::name() const
{
    return _name;
}

std::string Application::bundleIdentifier() const
{
    return _bundleIdentifier;
}

std::string Application::version() const
{
    return _version;
}

std::string Application::path() const
{
    return _path;
}

std::shared_ptr<ProvisioningProfile> Application::provisioningProfile()
{
	if (_provisioningProfile == NULL)
	{
		fs::path path(this->path());
		path.append("embedded.mobileprovision");

		_provisioningProfile = std::make_shared<ProvisioningProfile>(path.string());
	}

	return _provisioningProfile;
}

std::vector<std::shared_ptr<Application>> Application::appExtensions() const
{
	std::vector<std::shared_ptr<Application>> appExtensions;

	fs::path plugInsPath(this->path());
	plugInsPath.append("PlugIns");

	if (!fs::exists(plugInsPath))
	{
		return appExtensions;
	}

	for (auto& file : fs::directory_iterator(plugInsPath))
	{
		if (file.path().extension() != ".appex")
		{
			continue;
		}

		auto appExtension = std::make_shared<Application>(file.path().string());
		if (appExtension == nullptr)
		{
			continue;
		}

		appExtensions.push_back(appExtension);
	}

	return appExtensions;
}