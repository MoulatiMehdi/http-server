#include "MimeType.hpp"
#include <map>
#include <string>

const std::string MimeType::DEFAULT = "application/octet-stream";

MimeType::MimeType()
{
    MimeType::map["html"]     = "text/html";
    MimeType::map["htm"]      = "text/html";
    MimeType::map["shtml"]    = "text/html";
    MimeType::map["css"]      = "text/css";
    MimeType::map["xml"]      = "text/xml";
    MimeType::map["gif"]      = "image/gif";
    MimeType::map["jpeg jpg"] = "image/jpeg";
    MimeType::map["js"]       = "application/javascript";
    MimeType::map["atom"]     = "application/atom+xml";
    MimeType::map["rss"]      = "application/rss+xml";

    MimeType::map["mml"] = "text/mathml";
    MimeType::map["txt"] = "text/plain";
    MimeType::map["jad"] = "text/vnd.sun.j2me.app-descriptor";
    MimeType::map["wml"] = "text/vnd.wap.wml";
    MimeType::map["htc"] = "text/x-component";

    MimeType::map["avif"]     = "image/avif";
    MimeType::map["png"]      = "image/png";
    MimeType::map["svg svgz"] = "image/svg+xml";
    MimeType::map["tif tiff"] = "image/tiff";
    MimeType::map["wbmp"]     = "image/vnd.wap.wbmp";
    MimeType::map["webp"]     = "image/webp";
    MimeType::map["ico"]      = "image/x-icon";
    MimeType::map["jng"]      = "image/x-jng";
    MimeType::map["bmp"]      = "image/x-ms-bmp";

    MimeType::map["woff"]  = "font/woff";
    MimeType::map["woff2"] = "font/woff2";

    MimeType::map["jar war ear"] = "application/java-archive";
    MimeType::map["json"]        = "application/json";
    MimeType::map["hqx"]         = "application/mac-binhex40";
    MimeType::map["doc"]         = "application/msword";
    MimeType::map["pdf"]         = "application/pdf";
    MimeType::map["ps eps ai"]   = "application/postscript";
    MimeType::map["rtf"]         = "application/rtf";
    MimeType::map["m3u8"]        = "application/vnd.apple.mpegurl";
    MimeType::map["kml"]         = "application/vnd.google-earth.kml+xml";
    MimeType::map["kmz"]         = "application/vnd.google-earth.kmz";
    MimeType::map["xls"]         = "application/vnd.ms-excel";
    MimeType::map["eot"]         = "application/vnd.ms-fontobject";
    MimeType::map["ppt"]         = "application/vnd.ms-powerpoint";
    MimeType::map["odg"] = "application/vnd.oasis.opendocument.graphics";
    MimeType::map["odp"] = "application/vnd.oasis.opendocument.presentation";
    MimeType::map["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    MimeType::map["odt"] = "application/vnd.oasis.opendocument.text";
    MimeType::map["pptx"] =
        "application/"
        "vnd.openxmlformats-officedocument.presentationml.presentation";
    MimeType::map["xlsx"] =
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    MimeType::map["docx"] =
        "application/"
        "vnd.openxmlformats-officedocument.wordprocessingml.document";
    MimeType::map["wmlc"]        = "application/vnd.wap.wmlc";
    MimeType::map["wasm"]        = "application/wasm";
    MimeType::map["7z"]          = "application/x-7z-compressed";
    MimeType::map["cco"]         = "application/x-cocoa";
    MimeType::map["jardiff"]     = "application/x-java-archive-diff";
    MimeType::map["jnlp"]        = "application/x-java-jnlp-file";
    MimeType::map["run"]         = "application/x-makeself";
    MimeType::map["pl pm"]       = "application/x-perl";
    MimeType::map["prc pdb"]     = "application/x-pilot";
    MimeType::map["rar"]         = "application/x-rar-compressed";
    MimeType::map["rpm"]         = "application/x-redhat-package-manager";
    MimeType::map["sea"]         = "application/x-sea";
    MimeType::map["swf"]         = "application/x-shockwave-flash";
    MimeType::map["sit"]         = "application/x-stuffit";
    MimeType::map["tcl tk"]      = "application/x-tcl";
    MimeType::map["der pem crt"] = "application/x-x509-ca-cert";
    MimeType::map["xpi"]         = "application/x-xpinstall";
    MimeType::map["xhtml"]       = "application/xhtml+xml";
    MimeType::map["xspf"]        = "application/xspf+xml";
    MimeType::map["zip"]         = "application/zip";

    MimeType::map["bin exe dll"] = "application/octet-stream";
    MimeType::map["deb"]         = "application/octet-stream";
    MimeType::map["dmg"]         = "application/octet-stream";
    MimeType::map["iso img"]     = "application/octet-stream";
    MimeType::map["msi msp msm"] = "application/octet-stream";

    MimeType::map["mid midi kar"] = "audio/midi";
    MimeType::map["mp3"]          = "audio/mpeg";
    MimeType::map["ogg"]          = "audio/ogg";
    MimeType::map["m4a"]          = "audio/x-m4a";
    MimeType::map["ra"]           = "audio/x-realaudio";

    MimeType::map["3gpp 3gp"] = "video/3gpp";
    MimeType::map["ts"]       = "video/mp2t";
    MimeType::map["mp4"]      = "video/mp4";
    MimeType::map["mpeg mpg"] = "video/mpeg";
    MimeType::map["mov"]      = "video/quicktime";
    MimeType::map["webm"]     = "video/webm";
    MimeType::map["flv"]      = "video/x-flv";
    MimeType::map["m4v"]      = "video/x-m4v";
    MimeType::map["mng"]      = "video/x-mng";
    MimeType::map["asx asf"]  = "video/x-ms-asf";
    MimeType::map["wmv"]      = "video/x-ms-wmv";
    MimeType::map["avi"]      = "video/x-msvideo";
}

const std::string &MimeType::getContentType(const std::string &extension)
{
    iterator it = map.find(extension);

    if (it == map.end())
        return DEFAULT;
    return it->second;
}

MimeType::~MimeType()
{
}
