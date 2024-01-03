#include "nlohmann/json.hpp"
#include <curl/curl.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
using namespace std;


/*
cd "/Users/sachingupta/Desktop/THESTOCKIE/" && g++ -std=c++11 main.cpp -o main -lcurl && "/Users/sachingupta/Desktop/THESTOCKIE/"main
*/

using json = nlohmann::json;

namespace
{
    std::size_t callback(
        const char *in,
        std::size_t size,
        std::size_t num,
        std::string *out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}

json getStockInfo(std::string urlLocal)
{
    CURL *curl = curl_easy_init();

    // Set remote URL.
    curl_easy_setopt(curl, CURLOPT_URL, urlLocal.c_str());

    // Don't bother trying IPv6, which would increase DNS resolution time.
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // Don't wait forever, time out after 10 seconds.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

    // Follow HTTP redirects if necessary.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Response information.
    long httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Run our HTTP GET command, capture the HTTP response code, and clean up.
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    // sizeof()

    if (httpCode == 200)
    {
        // std::cout << "\nGot successful response from " << urlLocal << std::endl;
        json data = json::parse(*httpData.get());

        return data;
    }
    else
    {
        std::cout << "Couldn't GET from " << urlLocal << " - exiting" << std::endl;
        return 0;
    }
}

void compareStocks()
{
    std::string stockCode, url;
    stockCode = "";
    std::string url1 = "https://fmpcloud.io/api/v3/quote/";
    std::string url2 = "?apikey=8b08365b7e4766bc2a7cd6421b03b88d";
    int x;

    std::cout << "\n\nEnter Number of Stocks to Compare: ";
    std::cin >> x;
    std::string stockCodes[x];
    for (int i = 0; i < x; i++)
    {
        std::string tempCode;
        std::cout << "Enter Stock Code: ";
        std::cin >> tempCode;
        stockCodes[i] = tempCode;
        stockCode = stockCode + tempCode + ",";
    }

    url = url1 + stockCode + url2;

    json data1 = getStockInfo(url);

    json financialData[x];

    for (int i = 0; i < x; i++)
    {
        financialData[i] = getStockInfo("https://fmpcloud.io/api/v3/income-statement/" + stockCodes[i] + "?limit=10&apikey=8b08365b7e4766bc2a7cd6421b03b88d");
    }

    for (int i = 0; i < x; i++)
    {
        std::string symbol = data1[i].value("symbol", "not found");
        std::string name = data1[i].value("name", "not found");
        double price1 = data1[i].value("price", 0.00);
        double change = data1[i].value("change", 0.00);

        double revenue = financialData[i][0].value("revenue", 0.00);
        double grossProfit = financialData[i][0].value("grossProfit", 0.00);
        double operatingIncomeLoss = financialData[i][0].value("operatingIncome", 0.00);
        double netIncomeLoss = financialData[i][0].value("netIncome", 0.00);
        double EPS = financialData[i][0].value("eps", 0.00);

        // Print the values
        std::cout << "\n----------------------------------------------------------------\n";
        std::cout << symbol << " (" << name << ") \n";
        std::cout << "Stock Performance:\n";
        std::cout << "  Price: $" << price1 << std::endl;
        std::cout << "  Change From Start of Day: $" << change << std::endl
                  << std::endl;

        std::cout << "Financial Performance:" << std::endl;
        std::cout << "  Revenue: " << revenue / 1000000000 << " Billion USD" << std::endl;
        std::cout << "  Gross Profit: " << grossProfit / 1000000000 << " Billion USD" << std::endl;
        std::cout << "  Operating Income: " << operatingIncomeLoss / 1000000000 << " Billion USD" << std::endl;
        std::cout << "  Net Income: " << netIncomeLoss / 1000000000 << " Billion USD" << std::endl;
        std::cout << "  Earnings per Share (Basic): $" << EPS;
        std::cout << "\n----------------------------------------------------------------\n";
    }

    url1 = "https://fmpcloud.io/api/v3/rating/";
    url2 = "?apikey=8b08365b7e4766bc2a7cd6421b03b88d";
    std::string ratings[x];
    int numRatings[x];
    for (int i = 0; i < x; i++)
    {
        url = url1 + stockCodes[i] + url2;
        ratings[i] = getStockInfo(url)[0].value("rating", "Not found");
        if (ratings[i] == "S")
        {
            numRatings[i] = 7;
        }
        else if (ratings[i] == "S-")
        {
            numRatings[i] = 6;
        }
        else if (ratings[i] == "A+")
        {
            numRatings[i] = 5;
        }
        else if (ratings[i] == "A")
        {
            numRatings[i] = 4;
        }
        else if (ratings[i] == "A-")
        {
            numRatings[i] = 3;
        }
        else if (ratings[i] == "B+")
        {
            numRatings[i] = 2;
        }
        else if (ratings[i] == "B")
        {
            numRatings[i] = 1;
        }
        else if (ratings[i] == "B-")
        {
            numRatings[i] = 0;
        }
    }

    int n = sizeof(numRatings) / sizeof(numRatings[0]);
    std::sort(numRatings, numRatings + n);
    int temp;
    std::string output = "";

    for (int i = 0; i < x / 2; i++)
    {
        temp = numRatings[i];
        numRatings[i] = numRatings[x - i - 1];
        numRatings[x - i - 1] = temp;
    }

    std::string checkLetter;
    if (numRatings[0] == 7)
    {
        checkLetter = "S";
    }
    else if (numRatings[0] == 6)
    {
        checkLetter = "S-";
    }
    else if (numRatings[0] == 5)
    {
        checkLetter = "A+";
    }
    else if (numRatings[0] == 4)
    {
        checkLetter = "A";
    }
    else if (numRatings[0] == 3)
    {
        checkLetter = "A-";
    }
    else if (numRatings[0] == 2)
    {
        checkLetter = "B+";
    }
    else if (numRatings[0] == 1)
    {
        checkLetter = "B";
    }
    else if (numRatings[0] == 0)
    {
        checkLetter = "B-";
    }

    for (int i = 0; i < x; i++)
    {
        url = url1 + stockCodes[i] + url2;
        std::string tempRatings = getStockInfo(url)[0].value("rating", "Not found");
        if (tempRatings == checkLetter)
        {
            output = output + ", " + stockCodes[i];
        }
    }

    std::cout << "\nBased on current Stock Data, Stockie Recommends" << output << ",to make an investment in.\n\n";
}

void topTenRatingStocks()
{
    std::string nasdaqURL = "https://fmpcloud.io/api/v3/nasdaq_constituent?apikey=8b08365b7e4766bc2a7cd6421b03b88d";
    std::string url1 = "https://fmpcloud.io/api/v3/rating/";
    std::string url2 = "?apikey=8b08365b7e4766bc2a7cd6421b03b88d";
    std::string url;
    json nasdaqData = getStockInfo(nasdaqURL);
    int ratings[10];
    std::string symbolRating[10];
    int x = 0;
    for (int i = 0; i < 100; i++)
    {
        // Ratings range from (B-, B, B+, A-, A, A+, S-, S, S+)
        // letter to number   (0, 1, 2,  3,  4, 5,  6,  7, 8)
        std::string stockCode = nasdaqData[i].value("symbol", "not found");
        url = url1 + stockCode + url2;

        std::string tempRating = getStockInfo(url)[0].value("rating", "Not found");
        if (tempRating == "S+")
        {
            ratings[x] = i;
            symbolRating[x] = tempRating;
            x++;
        }
        else if (tempRating == "S")
        {
            ratings[x] = i;
            symbolRating[x] = tempRating;
            x++;
        }
        else if (tempRating == "S-")
        {
            ratings[x] = i;
            symbolRating[x] = tempRating;
            x++;
        }

        if (x == 10)
        {
            i = 1000;
        }
    }

    // Outputting Top Stocks
    std::cout << "Current Top Ten Highest Rated Stocks:" << std::endl;
    std::cout << "1. " << nasdaqData[ratings[0]].value("name", "not found") << " (" << nasdaqData[ratings[0]].value("symbol", "not found") << ", " << symbolRating[0] << ")" << std::endl;
    std::cout << "2. " << nasdaqData[ratings[1]].value("name", "not found") << " (" << nasdaqData[ratings[1]].value("symbol", "not found") << ", " << symbolRating[1] << ")" << std::endl;
    std::cout << "3. " << nasdaqData[ratings[2]].value("name", "not found") << " (" << nasdaqData[ratings[2]].value("symbol", "not found") << ", " << symbolRating[2] << ")" << std::endl;
    std::cout << "4. " << nasdaqData[ratings[3]].value("name", "not found") << " (" << nasdaqData[ratings[3]].value("symbol", "not found") << ", " << symbolRating[3] << ")" << std::endl;
    std::cout << "5. " << nasdaqData[ratings[4]].value("name", "not found") << " (" << nasdaqData[ratings[4]].value("symbol", "not found") << ", " << symbolRating[4] << ")" << std::endl;
    std::cout << "6. " << nasdaqData[ratings[5]].value("name", "not found") << " (" << nasdaqData[ratings[4]].value("symbol", "not found") << ", " << symbolRating[5] << ")" << std::endl;
    std::cout << "7. " << nasdaqData[ratings[6]].value("name", "not found") << " (" << nasdaqData[ratings[6]].value("symbol", "not found") << ", " << symbolRating[6] << ")" << std::endl;
    std::cout << "8. " << nasdaqData[ratings[7]].value("name", "not found") << " (" << nasdaqData[ratings[7]].value("symbol", "not found") << ", " << symbolRating[7] << ")" << std::endl;
    std::cout << "9. " << nasdaqData[ratings[8]].value("name", "not found") << " (" << nasdaqData[ratings[8]].value("symbol", "not found") << ", " << symbolRating[8] << ")" << std::endl;
    std::cout << "10. " << nasdaqData[ratings[9]].value("name", "not found") << " (" << nasdaqData[ratings[9]].value("symbol", "not found") << ", " << symbolRating[9] << ")" << std::endl;
}

void topTenGainers()
{
    std::string url = "https://fmpcloud.io/api/v3/gainers?apikey=8b08365b7e4766bc2a7cd6421b03b88d";

    json data = getStockInfo(url);

    //Printing out the stocks

    std::cout << "\n\nTop 10 stocks in Change Percentage: \n";
    for (int i = 0; i < 10; i++)
    {
        std::cout << "    " << i + 1 << ". " << data[i].value("ticker", "not found") << " (" << data[i].value("companyName", "not found") << " " << data[i].value("changesPercentage", "not found") << "%) \n";
    }
}

void rating()
{
    //Getting the stock information, using the API

    std::string stockCode, url;
    std::string url1 = "https://fmpcloud.io/api/v3/rating/";
    std::string url2 = "?apikey=8b08365b7e4766bc2a7cd6421b03b88d";
    std::cout << "\n\n\nEnter Stock Code: ";
    std::cin >> stockCode;

    url = url1 + stockCode + url2;

    json ratings = getStockInfo(url);

    string symbol = ratings[0].value("symbol", "not found");

    string ratingRecommendation = ratings[0].value("ratingRecommendation", "not found");
    while (ratingRecommendation.length() <= 13)
    {
        ratingRecommendation.push_back(' ');
    }
    string rating = ratings[0].value("rating", "not found");
    while (rating.length() <= 13)
    {
        rating.push_back(' ');
    }
    double ratingScore = ratings[0].value("ratingScore", 0.00);

    //Making the recommendations a length of 13 for better formatting

    string ratingDetailsDCFRecommendation = ratings[0].value("ratingDetailsDCFRecommendation", "not found");
    while (ratingDetailsDCFRecommendation.length() <= 13)
    {
        ratingDetailsDCFRecommendation.push_back(' ');
    }
    string ratingDetailsROERecommendation = ratings[0].value("ratingDetailsROERecommendation", "not found");
    while (ratingDetailsROERecommendation.length() <= 13)
    {
        ratingDetailsROERecommendation.push_back(' ');
    }
    string ratingDetailsROARecommendation = ratings[0].value("ratingDetailsROARecommendation", "not found");
    while (ratingDetailsROARecommendation.length() <= 13)
    {
        ratingDetailsROARecommendation.push_back(' ');
    }
    string ratingDetailsDERecommendation = ratings[0].value("ratingDetailsDERecommendation", "not found");
    while (ratingDetailsDERecommendation.length() <= 13)
    {
        ratingDetailsDERecommendation.push_back(' ');
    }
    string ratingDetailsPERecommendation = ratings[0].value("ratingDetailsPERecommendation", "not found");
    while (ratingDetailsPERecommendation.length() <= 13)
    {
        ratingDetailsPERecommendation.push_back(' ');
    }
    string ratingDetailsPBRecommendation = ratings[0].value("ratingDetailsPBRecommendation", "not found");
    while (ratingDetailsPBRecommendation.length() <= 13)
    {
        ratingDetailsPBRecommendation.push_back(' ');
    }

    //Getting the scores and storing them as variables

    double ratingDetailsDCFScore = ratings[0].value("ratingDetailsDCFScore", 0.00);
    double ratingDetailsROEScore = ratings[0].value("ratingDetailsROEScore", 0.00);
    double ratingDetailsROAScore = ratings[0].value("ratingDetailsROAScore", 0.00);
    double ratingDetailsDEScore = ratings[0].value("ratingDetailsDEScore", 0.00);
    double ratingDetailsPEScore = ratings[0].value("ratingDetailsPEScore", 0.00);
    double ratingDetailsPBScore = ratings[0].value("ratingDetailsPBScore", 0.00);

    // Print the values
    std::cout << "\n\n\nFor " << symbol << ":\n\n\n";
    std::cout << "| CATEGORY               | RECOMMENDATION | SCORE\n";
    std::cout << "_________________________________________________\n\n";

    std::cout << "|"
              << " DCF                    | " << ratingDetailsDCFRecommendation << " | " << ratingDetailsDCFScore << "\n";
    std::cout << "|"
              << " ROE                    | " << ratingDetailsROERecommendation << " | " << ratingDetailsROEScore << "\n";
    std::cout << "|"
              << " ROA                    | " << ratingDetailsROARecommendation << " | " << ratingDetailsROAScore << "\n";
    std::cout << "|"
              << " DE                     | " << ratingDetailsDERecommendation << " | " << ratingDetailsDEScore << "\n";
    std::cout << "|"
              << " PE                     | " << ratingDetailsPERecommendation << " | " << ratingDetailsPEScore << "\n";
    std::cout << "|"
              << " PB                     | " << ratingDetailsPBRecommendation << " | " << ratingDetailsPBScore << "\n";

    double totalpoints = ratingDetailsDCFScore + ratingDetailsROEScore + ratingDetailsROAScore + ratingDetailsDEScore + ratingDetailsPEScore + ratingDetailsPBScore;

    std::cout << "\n|"
              << " OVERALL                | " << ratingRecommendation << " | " << ratingScore << "\n";
    std::cout << "|"
              << "                        | " << rating << " | " << totalpoints << "/30\n";

    std::cout << "\n\n\n";
}

void learn()
{
    //Gives information on business terms used in the rating feature

    cout << "\n\n\nDCF (Discounted Cash Flow) is way to value investments by looking at how well is generates cash flow for its investors.\n\n";
    cout << "ROE (Return on Equity) looks at the relationship between a companies profit and investor return. As investors, it helps us know that we will make profit by investing in a company.\n\n";
    cout << "ROA (Retun on Assets) looks at a companies profits relative to their resources and assets.\n\n";
    cout << "DE is the Debt to Equity ratio and helps to determine if a company can pay off its liabilities.\n\n";
    cout << "PE is the Price to Earnings ratio for a stock. It helps determine its value, as a very high PE may mean the stock is overvalued, so may drop in price shortly.\n\n";
    cout << "PB is Price to Book ratio, which measures the company's valuation compared to the stock value.\n\n\n";
}

void visual()
{
    //Getting the stock information
    std::string stockCode, url;
    std::string url1 = "https://fmpcloud.io/api/v3/historical-chart/1hour/";
    std::string url2 = "?apikey=cd7cae9891b98797119cfbb0a3e0d50b";
    std::cout << "\n\n\nEnter Stock Code: ";
    std::cin >> stockCode;

    url = url1 + stockCode + url2;

    json data = getStockInfo(url);

    int sizeofgraph;
    cout << "\n\nWhat should the size of the graph be (recommended to be 36): ";
    cin >> sizeofgraph;
    cout << "\n\n";
    double doubsize = sizeofgraph;

    double low = data[0].value("close", 0.00);
    double high = 0.00;

    for (int i = 0; i < sizeofgraph; i++)
    {
        double price = data[i].value("close", 0.00);
        if (price >= high)
        {
            high = price;
        }
        if (price <= low)
        {
            low = price;
        }
    }

    double factor = (high - low) / doubsize;

    string array[sizeofgraph][sizeofgraph];
    for (int a = 0; a < sizeofgraph; a++)
    {
        for (int b = 0; b < sizeofgraph; b++)
        {
            array[a][b] = " ";
        }
    }

    for (int q = 0; q < sizeofgraph; q++)
    {
        int position = 0;
        double price = data[q].value("close", 0.00); // value being tested (at index i) for price
        price = price - low;

        if (price > factor)
        {
            while (price > factor)
            {
                price = price - factor;
                position = position + 1;
            }
        }
        else
        {
            position = 0;
        }
        array[(sizeofgraph - 1) - position][q] = "*";
    }
    cout << "\n\n\n"
         << high << "\n";

    int j;
    for (int start = 0; start < sizeofgraph; start++)
    {
        for (int i = 0, j = sizeofgraph - 1; i < sizeofgraph / 2; i++, j--)
        {
            string temp = array[start][i];
            array[start][i] = array[start][j];
            array[start][j] = temp;
        }
    }

    //Formatting and outputting the graph to the user's screen

    for (int aa = 0; aa < sizeofgraph; aa++)
    {
        cout << "  | ";
        for (int bb = 0; bb < sizeofgraph; bb++)
        {
            cout << array[aa][bb] << " ";
        }
        cout << "\n";
    }
    cout << "  ";
    for (int rr = 0; rr <= (2 * sizeofgraph); rr++)
    {
        cout << "-";
    }
    cout << "\n"
         << low;

    string StartDate = data[sizeofgraph - 1].value("date", "not found");
    string EndDate = data[0].value("date", "not found");

    cout << "\n\n\n\n\n"
         << "LOW: " << low << "    HIGH: " << high << "    FACTOR: " << factor;
    cout << "\n"
         << "Start Date: " << StartDate << "     End Date: " << EndDate;
    cout << "\n\n\n\n\n";

    // low, high = factor
    // position (if a very low number, will be at bottom of chart)
}

void getStockInfo()
{
    std::string stockCode, url;
    std::string url1 = "https://fmpcloud.io/api/v3/quote/";
    std::string url2 = "?apikey=8b08365b7e4766bc2a7cd6421b03b88d";
    std::cout << "Enter Stock Code: ";
    std::cin >> stockCode;

    url = url1 + stockCode + url2;

    json data1 = getStockInfo(url);

    std::string symbol = data1[0].value("symbol", "not found");
    std::string name = data1[0].value("name", "not found");
    double price1 = data1[0].value("price", 0.00);
    double change = data1[0].value("change", 0.00);

    json financialData = getStockInfo("https://fmpcloud.io/api/v3/income-statement/" + stockCode + "?limit=10&apikey=8b08365b7e4766bc2a7cd6421b03b88d");

    double revenue = financialData[0].value("revenue", 0.00);
    double grossProfit = financialData[0].value("grossProfit", 0.00);
    double operatingIncomeLoss = financialData[0].value("operatingIncome", 0.00);
    double netIncomeLoss = financialData[0].value("netIncome", 0.00);
    double EPS = financialData[0].value("eps", 0.00);

    // Print the values
    std::cout << "\n----------------------------------------------------------------\n";
    std::cout << symbol << " (" << name << ") \n";
    std::cout << "Stock Performance:\n";
    std::cout << "  Price: $" << price1 << std::endl;
    std::cout << "  Change From Start of Day: $" << change << std::endl
              << std::endl;

    std::cout << "Financial Performance:" << std::endl;
    std::cout << "  Revenue: " << revenue / 1000000000 << " Billion USD" << std::endl;
    std::cout << "  Gross Profit: " << grossProfit / 1000000000 << " Billion USD" << std::endl;
    std::cout << "  Operating Income: " << operatingIncomeLoss / 1000000000 << " Billion USD" << std::endl;
    std::cout << "  Net Income: " << netIncomeLoss / 1000000000 << " Billion USD" << std::endl;
    std::cout << "  Earnings per Share (Basic): $" << EPS;
    std::cout << "\n----------------------------------------------------------------\n";
}

int main()
{
    //Calling the functions so the user can use the features

    std::string N;

    std::cout << "\nWelcome To Stockie, your investing guide!";
    std::cout << "\n\nEnter a Letter to Execute a Function:\n   a) Compare Number of Stocks\n   b) Display All Information of Stock\n   c) See Stock Rating\n   d) Top 10 Rated Stocks\n   e) Top 10 stocks in Change Percentage\n   f) Price Graph\n   learn) Business Terms and their Meanings\nEnter a Letter (type exit to exit program): ";
    std::cin >> N;
    std::cout << "\n\n\n";

    if (N == "a")
    {
        compareStocks();
    }
    else if (N == "b")
    {
        getStockInfo();
    }
    else if (N == "c")
    {
        rating();
    }
    else if (N == "d")
    {
        topTenRatingStocks();
    }
    else if (N == "e")
    {
        topTenGainers();
    }
    else if (N == "f")
    {
        visual();
    }
    else if (N == "learn")
    {
        learn();
    }
    else
    {
        return 0;
    }
    cout << "\n\n\n";
    main();
    return 0;
}
