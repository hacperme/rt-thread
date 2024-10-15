#include <rtthread.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "sha256.h"
#include "hmac_sha256.h"
#include "at_client_ssl.h"
#include "sha256.h"

#define AWS_ACCESS_KEY_ID "xxxxxxxxxxxxxx"
#define AWS_SECRET_ACCESS_KEY "xxxxxxxxxxxxxxxxxxxxx"
#define AWS_REGION "ap-east-1"
#define AWS_SERVICE "s3"
#define AWS_BUCKET "iot-encortec"
#define AWS_HOST "iot-encortec.s3.ap-east-1.amazonaws.com"

void bin2hex(const uint8_t *src, uint32_t src_length, uint8_t *output)
{
    uint8_t *p = output;
    for (uint32_t i = 0; i < src_length; i++) {
        snprintf(p + (i*2), 3, "%02x", src[i]);
    }
}

void create_hashed_payload(uint8_t *payload, uint32_t payload_length, uint8_t *output)
{
    Sha256Context ctx;
    SHA256_HASH digest;

    Sha256Initialise(&ctx);
    Sha256Update(&ctx, payload, payload_length);
    Sha256Finalise(&ctx, &digest);

    bin2hex(digest.bytes, 32, output);
}

void create_hashed_canonical_request(const char *method, const char *uri, const char *localtime, const char *amz_date, const char *content_sha256, const char *host, int content_length, uint8_t *output)
{
    char canonical_request[512] = {0};
    char header_value[128] = {0};

    // HTTPMethod
    strcat(canonical_request, method);
    strcat(canonical_request, "\n");

    // CanonicalURI
    strcat(canonical_request, uri);
    strcat(canonical_request, "\n");

    // CanonicalQueryString
    strcat(canonical_request, "\n");
    
    // CanonicalHeaders
    memset(header_value, 0, sizeof(header_value));
    snprintf(header_value, sizeof(header_value), "host:%s\n", host);
    strcat(canonical_request, header_value);

    memset(header_value, 0, sizeof(header_value));
    snprintf(header_value, sizeof(header_value), "x-amz-acl:%s\n", "public-read-write");
    strcat(canonical_request, header_value);

    memset(header_value, 0, sizeof(header_value));
    snprintf(header_value, sizeof(header_value), "x-amz-content-sha256:%s\n", content_sha256);
    strcat(canonical_request, header_value);

    memset(header_value, 0, sizeof(header_value));
    snprintf(header_value, sizeof(header_value), "x-amz-date:%s\n", amz_date);
    strcat(canonical_request, header_value);

    strcat(canonical_request, "\n");

    // SignedHeaders
    strcat(canonical_request, "host;x-amz-acl;x-amz-content-sha256;x-amz-date\n");

    // HashedPayload
    strcat(canonical_request, content_sha256);

    rt_kprintf("--------- canonical request:\n");
    rt_kprintf("%s\n", canonical_request);
    rt_kprintf("---------\n");

    Sha256Context ctx;
    SHA256_HASH digest;

    Sha256Initialise(&ctx);
    Sha256Update(&ctx, canonical_request, strlen(canonical_request));
    Sha256Finalise(&ctx, &digest);
    
    bin2hex(digest.bytes, 32, output);
}

void create_signature(const char *amz_date, const char *date, const char *region, const char *service, 
                      char *hashed_canonical_request, char *output)
{
    char string_to_sign[512] = {0};

    snprintf(
        string_to_sign,
        256,
        "AWS4-HMAC-SHA256\n%s\n%s/%s/%s/aws4_request\n%s", 
        amz_date, 
        date,
        region, 
        service, 
        hashed_canonical_request
    );

    rt_kprintf("--------- string to sign:\n");
    rt_kprintf("%s\n", string_to_sign);
    rt_kprintf("---------\n");

    uint8_t access_key[64] = {0};
    uint8_t date_key[32] = {0};
    uint8_t date_region_key[32] = {0};
    uint8_t date_region_service_key[32] = {0};
    uint8_t signing_key[32] = {0};
    uint8_t signature[32] = {0};

    // DateKey = HMAC-SHA256("AWS4"+"<SecretAccessKey>", "<YYYYMMDD>")
    snprintf(access_key, 128, "AWS4%s", AWS_SECRET_ACCESS_KEY);
    hmac_sha256(access_key, strlen(access_key), date, strlen(date), &date_key, sizeof(date_key));

    // DateRegionKey = HMAC-SHA256(<DateKey>, "<aws-region>")
    hmac_sha256(date_key, sizeof(date_key), region, strlen(region), &date_region_key, sizeof(date_region_key));

    // DateRegionServiceKey = HMAC-SHA256(<DateRegionKey>, "<aws-service>")
    hmac_sha256(date_region_key, sizeof(date_region_key), service, strlen(service), &date_region_service_key, sizeof(date_region_service_key));

    // SigningKey = HMAC-SHA256(<DateRegionServiceKey>, "aws4_request")
    hmac_sha256(date_region_service_key, sizeof(date_region_service_key), "aws4_request", strlen("aws4_request"), &signing_key, sizeof(signing_key));

    hmac_sha256(signing_key, sizeof(signing_key), string_to_sign, strlen(string_to_sign), &signature, sizeof(signature));
    bin2hex(signature, sizeof(signature), output);
}

char weekDay[7][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char mon[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#define QTH_AWS_X509_CA "-----BEGIN CERTIFICATE-----\r\n" \
                    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n" \
                    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n" \
                    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n" \
                    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n" \
                    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n" \
                    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n" \
                    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n" \
                    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n" \
                    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n" \
                    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n" \
                    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n" \
                    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n" \
                    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n" \
                    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n" \
                    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n" \
                    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n" \
                    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n" \
                    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n" \
                    "-----END CERTIFICATE-----"

#define QST_SSL_CA "-----BEGIN CERTIFICATE-----\r\n" \
				"MIIEtDCCA5ygAwIBAgIJAL/pgY5ZFAJZMA0GCSqGSIb3DQEBCwUAMIGMMQswCQYD\r\n" \
				"VQQGEwJDTjEOMAwGA1UECBMFQW5IdWkxDjAMBgNVBAcTBUhlRmVpMRAwDgYDVQQK\r\n" \
				"EwdRdWVjdGVsMQswCQYDVQQLEwJTVDEWMBQGA1UEAxMNMTEyLjMxLjg0LjE2NDEm\r\n" \
				"MCQGCSqGSIb3DQEJARYXam9tYW4uamlhbmdAcXVlY3RlbC5jb20wHhcNMjIwMjA5\r\n" \
				"MDYwOTAwWhcNMzIwMjA3MDYwOTAwWjCBjDELMAkGA1UEBhMCQ04xDjAMBgNVBAgT\r\n" \
				"BUFuSHVpMQ4wDAYDVQQHEwVIZUZlaTEQMA4GA1UEChMHUXVlY3RlbDELMAkGA1UE\r\n" \
				"CxMCU1QxFjAUBgNVBAMTDTExMi4zMS44NC4xNjQxJjAkBgkqhkiG9w0BCQEWF2pv\r\n" \
				"bWFuLmppYW5nQHF1ZWN0ZWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\r\n" \
				"CgKCAQEAxPA3lJd/HwTtgDu8tgOIemI6LiFySsDhZvZf3jh/zatJkb6U8AdTLbHc\r\n" \
				"r/aDUgEKhzpxTfuB4JLRqF+/geKq9k89ulmmC+1V7AK1TihR+aVXNsbSuXSMaN3m\r\n" \
				"zfNH42WE/4EgaASGrZaHaWh33Cv6VUAq6O9SVgMrdvPn4fjvk+py69nQ++qvsGFK\r\n" \
				"8i1GOsi6wv69v+f+o0IfrDkQpVx/1+RBmL981ZpjAOBDNaJherx845ZLHTOTPXhJ\r\n" \
				"7XFMGbBtSRnjJtGGtWk028FE84UDnQ2CkAhb1VJvbxUeGhxqCxBSEaSqjPBh7UO1\r\n" \
				"rAXQ+0VBsM2u4tJDskTpIo09x/oKwQIDAQABo4IBFTCCAREwDAYDVR0TBAUwAwEB\r\n" \
				"/zAdBgNVHQ4EFgQUdgLJhI16sNJZF9awAJkeb3TBMW8wgcEGA1UdIwSBuTCBtoAU\r\n" \
				"dgLJhI16sNJZF9awAJkeb3TBMW+hgZKkgY8wgYwxCzAJBgNVBAYTAkNOMQ4wDAYD\r\n" \
				"VQQIEwVBbkh1aTEOMAwGA1UEBxMFSGVGZWkxEDAOBgNVBAoTB1F1ZWN0ZWwxCzAJ\r\n" \
				"BgNVBAsTAlNUMRYwFAYDVQQDEw0xMTIuMzEuODQuMTY0MSYwJAYJKoZIhvcNAQkB\r\n" \
				"Fhdqb21hbi5qaWFuZ0BxdWVjdGVsLmNvbYIJAL/pgY5ZFAJZMAsGA1UdDwQEAwIB\r\n" \
				"BjARBglghkgBhvhCAQEEBAMCAAcwDQYJKoZIhvcNAQELBQADggEBAB2eFfablKpx\r\n" \
				"uM3xaZ4RmGjjBFclPKDdW/qGNMeCq1IvBZ5Hd7R/FjUT3STojUFAMW+LRwbV5v7f\r\n" \
				"RQGZmlavQQNmA75Clwt9XD8oob9KnfMu8JQ9RB99xptu9DfYCxXYTFavSjzN3Tkg\r\n" \
				"i8d/6itOQI0ISQtvtOKhCbzT2Nrv5meegOrB3utdpEZ5FkHGmdVPTnj2bhRUPCYR\r\n" \
				"lLXxvA2+Rcaw7LcgIIdsv47rFCsLO9bkaCNAvGw8LUVEP3TbtvxfW6uKjrMErUox\r\n" \
				"m5Ny+VnmlWN7Kcb8o4jwkwTXPa57fumkhRorgWO5f6+A8gjHjfpWK8Ew8xfd3T1W\r\n" \
				"Xh270MLwxk4=\r\n" \
				"-----END CERTIFICATE-----"


int at_https_upload_file(const char *filename)
{
    char hashed_payload[65] = {0};
    char hashed_canonical_request[65] = {0};
    char signature[65] = {0};
    char auth[512] = {0};

    FILE *file = NULL;
    file = fopen(filename, "rb");
    if (!file) {
        rt_kprintf("Failed to open file: %s\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
	long file_length = ftell(file);
	fseek(file, 0, SEEK_SET);

    int bytes_read = -1;
    rt_uint8_t buffer[1024] = {0};

    Sha256Context ctx;
    SHA256_HASH digest;

    Sha256Initialise(&ctx);
    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        Sha256Update(&ctx, buffer, bytes_read);
    }
    Sha256Finalise(&ctx, &digest);
    bin2hex(digest.bytes, 32, hashed_payload);
    rt_kprintf("hashed_payload: %s\n", hashed_payload);
    fseek(file, 0, SEEK_SET);

    char date[9] = {0};  // "20130524";
    char amz_date[17] = {0};  // "20130524T220855Z";
    char localtime[64] = {0};  // Fri, 24 May 2013 00:00:00 GMT
    char *host = AWS_HOST;
    int content_length = file_length;
    char *region = AWS_REGION;
    char *service = AWS_SERVICE;
    char uri[128] = {0};

    // 构建时间字符串标头
    time_t rawtime;
    time(&rawtime);
    rt_kprintf("raw time: %d\n", rawtime);
    struct tm *utc_time = gmtime(&rawtime);
    rt_kprintf(
        "utc time: %04d-%02d-%02d %02d:%02d:%02d\n",
        utc_time->tm_year+1900, utc_time->tm_mon+1, utc_time->tm_mday, utc_time->tm_hour, utc_time->tm_min, utc_time->tm_sec
    );
    
    // date
    snprintf(date, sizeof(date), "%04d%02d%02d", utc_time->tm_year+1900, utc_time->tm_mon+1, utc_time->tm_mday);
    rt_kprintf("date: %s\n", date);
    
    // amzdate
    snprintf(amz_date, sizeof(amz_date), "%04d%02d%02dT%02d%02d%02dZ",
            utc_time->tm_year+1900, utc_time->tm_mon+1, utc_time->tm_mday, utc_time->tm_hour, utc_time->tm_min, utc_time->tm_sec);
    rt_kprintf("amz_date: %s\n", amz_date);
    
    // localtime
    snprintf(localtime, sizeof(localtime), "%s, %02d %s %04d %02d:%d:%02d GMT", weekDay[utc_time->tm_wday], utc_time->tm_mday, mon[utc_time->tm_mon],
            utc_time->tm_year+1900, utc_time->tm_hour, utc_time->tm_min, utc_time->tm_sec);
    rt_kprintf("localtime: %s\n", localtime);

    // uri
    strcat(uri, "/");
    strcat(uri, filename);
    rt_kprintf("uri: %s\n", uri);
    
    // 创建规范请求hash值
    create_hashed_canonical_request("PUT", uri, localtime, amz_date, hashed_payload, host, content_length, hashed_canonical_request);
    rt_kprintf("hashed_canonical_request: %s\n", hashed_canonical_request);

    // 创建签名
    create_signature(amz_date, date, region, service, hashed_canonical_request, signature);
    rt_kprintf("signature: %s\n", signature);

    // 构建请求头Authentication
    snprintf(
        auth,
        512,
        "AWS4-HMAC-SHA256 Credential=%s/%s/%s/%s/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date;x-amz-acl,Signature=%s",
        AWS_ACCESS_KEY_ID, date, region, service, signature
    );
    rt_kprintf("Authorization: %s\n", auth);

    char request[1024] = {0};
    snprintf(
        request,
        1024,
        "PUT %s HTTP/1.1\r\n" \
        "Host: %s\r\n" \
        "Date: %s\r\n" \
        "Content-Length: %d\r\n" \
        "Authorization: %s\r\n" \
        "X-Amz-Date: %s\r\n" \
        "X-Amz-Content-Sha256: %s\r\n" \
        "X-Amz-Acl: public-read-write\r\n\r\n",
        uri, host, localtime, content_length, auth, amz_date, hashed_payload
    );

    at_client_t client1 = at_client_get("uart1");

	if(at_ssl_cacert_save(client1, "cacert_st.pem", QST_SSL_CA, strlen(QST_SSL_CA)) != true) {
        fclose(file);
        return -1;
    }

    if(at_ssl_connect(client1, "cacert_st.pem", AWS_HOST, 443, strlen(request) + content_length) != true) {
        fclose(file);
        return -1;
    }

	if(at_ssl_send(client1, request, strlen(request)) != true) {
        fclose(file);
        at_ssl_close(client1);
        return -1;
    }

    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        if(at_ssl_send(client1, buffer, bytes_read) != true) {
            fclose(file);
            at_ssl_close(client1);
            return -1;
        }
    }

    fclose(file);
    bool result = at_ssl_check(client1);
    at_ssl_close(client1);

    return result ? 0 : -1;
}
