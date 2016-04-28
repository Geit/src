
#include "webAPI.h"

// if status == success, returns "success", or slotName's contents if specified...
// otherwise returns the "message" if no success
std::string webAPI::simplePost(std::string endpoint, std::string data, std::string slotName)
{
	// declare our output and go ahead and attempt to get data from remote
	nlohmann::json response = request(endpoint, data, 1);
	std::string output;

	// if we got data back...
	if (response.count("status") && response["status"].get<std::string>() == "success")
	{
		// use custom slot if specified (not "")
		if (!(slotName.empty()) && response.count(slotName))
		{
			output = response[slotName].get<std::string>();
		}
		else
		{
			output = "success";
		}
	} 
	else //default message is an error, the other end always assumes "success" or the specified slot
	{
		if (response.count("message"))
		{
			output = response["message"].get<std::string>();
		}
		else
		{
			output = "Message not provided by remote.";
		}
	}

	return output;				
}

// this can be broken out to separate the json bits later if we need raw or other http type requests
// all it does is fetch via get or post, and if the status is 200 returns the json, else error json
nlohmann::json webAPI::request(std::string endpoint, std::string data, int reqType)
{
	nlohmann::json response;

	if (!endpoint.empty()) //data is allowed to be an empty string if we're doing a normal GET
	{
		CURL *curl = curl_easy_init(); // start up curl

                if (curl)
                {
                        std::string readBuffer; // container for the remote response
			long http_code = 0; // we get this after performing the get or post

                        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str()); // endpoint is always specified by caller
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // place the data into readBuffer using writeCallback
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // specify readBuffer as the container for data

			if (reqType == 1)
			{
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
			}

			CURLcode res = curl_easy_perform(curl); // make the request!
			
			curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code); //get status code

			if (res == CURLE_OK && http_code == 200 && !(readBuffer.empty())) // check it all out and parse
			{
                        	response = nlohmann::json::parse(readBuffer);
			}
			curl_easy_cleanup(curl); // always wipe our butt
		}
		else //default err messages below
		{
        		response["message"] = "Failed to initialize cURL.";
        		response["status"] = "failure";
		}
	}
	else
	{
        	response["message"] = "Invalid endpoint URL.";
        	response["status"] = "failure";
	}

	return response;
}

// This is used by curl to grab the response and put it into a var
size_t webAPI::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
}

