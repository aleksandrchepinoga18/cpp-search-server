-----------------------------------------------------------------------
main.cpp
-----------------------------------------------------------------------
#include "document.h"
#include "paginator.h"
#include "read_input_functions.h"
#include "string_processing.h"
#include "search_server.h"
#include "request_queue.h"

using namespace std;
 
int main() {
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server);
 
    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});
 
    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("curly dog"s);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("big collar"s);
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("sparrow"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
}
-----------------------------------------------------------------------
document.cpp
-----------------------------------------------------------------------



-----------------------------------------------------------------------
document.h
-----------------------------------------------------------------------
#pragma once
#include <iostream>
 
struct Document {
    Document() = default;
    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    } 
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};
 
enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};
-----------------------------------------------------------------------
paginator.h
-----------------------------------------------------------------------
#pragma once
#include <algorithm>
#include <vector>
#include <cassert>
#include <iostream>
#include "document.h"

using namespace std::string_literals;
 
template <typename IteratorRanges>
class IteratorRange {
public:
    explicit IteratorRange(IteratorRanges begin, IteratorRanges end) : begin_(begin), end_(end), size_(distance(begin, end)){}
    IteratorRanges begin() const  {
        return begin_;
    }
    IteratorRanges end() const {
        return end_;
    }
    size_t size() const {
        return size_;
    }
private:
    IteratorRanges begin_;
    IteratorRanges end_;
    size_t size_;
};

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ document_id = "s << document.id 
        << ", relevance = "s << document.relevance 
        << ", rating = "s << document.rating << " }"s;
    return out;
}

template <typename To_Out>
std::ostream& operator<<(std::ostream& out, const IteratorRange<To_Out>& sheet) {
    auto helper = sheet.begin();
    while(helper != sheet.end()){
        out<< *helper;
        //out<< "Test";
        ++helper;
    }
    return out;
}
  
template <typename Paginatorr>
class Paginator {
public :
    Paginator(const Paginatorr& result_begin, const Paginatorr& result_end, size_t size_of_sheet){
        auto full_size = distance(result_begin, result_end);
        Paginatorr helper = result_begin;
        for (auto i = 0; i < full_size/size_of_sheet; ++i) {
            sheets.push_back(IteratorRange<Paginatorr>(helper, helper+ size_of_sheet));
            helper=helper+size_of_sheet;
        }
        if (helper!= result_end) {
            sheets.push_back(IteratorRange<Paginatorr>(helper, result_end));
        }
    }
    auto begin() const {
        return sheets.begin();
    }
    auto end() const {
        return sheets.end();
    }
    size_t size() {
        return sheets.size();
    }
private: 
    std::vector<IteratorRange<Paginatorr>> sheets;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
-----------------------------------------------------------------------
read_input_functions.cpp
-----------------------------------------------------------------------
#include "read_input_functions.h"
#include <iostream>

using namespace std::string_literals;

std::string ReadLine() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}
-----------------------------------------------------------------------
read_input_functions.h
-----------------------------------------------------------------------
#pragma once
#include "document.h"
#include <string>
 
std::string ReadLine(); 
int ReadLineWithNumber(); 
-----------------------------------------------------------------------
request_queue.cpp
-----------------------------------------------------------------------
#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [&status](int document_id, DocumentStatus document_status, int rating) {return document_status == status;});
}
    
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}
    
int RequestQueue::GetNoResultRequests() const {
    return count_if(requests_.begin(), requests_.end(), [](QueryResult query){return query.query_result == 0;});
}
-----------------------------------------------------------------------
request_queue.h
-----------------------------------------------------------------------
#pragma once
#include "search_server.h"
#include <deque>
 
class RequestQueue {
public:
    RequestQueue(const SearchServer& search_server) : search_request(search_server){}   
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        bool query_result;
    };
    const SearchServer& search_request;
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
}; 

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    std::vector<Document> helper = search_request.FindTopDocuments(raw_query, document_predicate);

    QueryResult query;
    query.query_result = (helper.empty()==false);

    if (!(requests_.size() < min_in_day_)){
        requests_.pop_front();
    } 
    requests_.push_back(query); 
    return helper;
}
-----------------------------------------------------------------------
search_server.cpp
-----------------------------------------------------------------------
#include <numeric>
#include "search_server.h"

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,const std::vector<int>& ratings){
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
}
 
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}
 
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}
 
bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(begin(ratings), end(ratings), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + text + " is invalid");
    }

    return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query result;
    for (const std::string& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
-----------------------------------------------------------------------
search_server.h
-----------------------------------------------------------------------
#pragma once
#include <tuple>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "read_input_functions.h"
#include "string_processing.h" 

using namespace std::string_literals;
 
class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words);
    SearchServer(const std::string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)){}
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;
    
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;
    int GetDocumentCount() const;
    int GetDocumentId(int index) const;
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const double EPSILON = 1e-6;
    const int MAX_RESULT_DOCUMENT_COUNT = 5;
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;

    bool IsStopWord(const std::string& word) const;
    static bool IsValidWord(const std::string& word);
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string& text) const;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const;
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)){
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
    const auto query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, document_predicate);
    sort(matched_documents.begin(), matched_documents.end(),
    [this](const Document& lhs, const Document& rhs) {
        if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
        return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
       if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}
-----------------------------------------------------------------------
string_processing.cpp
-----------------------------------------------------------------------
#include "string_processing.h"
 
std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}
-----------------------------------------------------------------------
string_processing.h
-----------------------------------------------------------------------
#pragma once
#include <set>
#include <vector>
#include <string>
 
std::vector<std::string> SplitIntoWords(const std::string& text);
 
template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}
