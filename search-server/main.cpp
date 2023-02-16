// -------- Начало модульных тестов поисковой системы ----------
 
// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
 
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}
/*
Разместите код остальных тестов здесь
*/
void TestMatchQuery(){
    string query1 = "what happend with this city";
    string query2 = "i fucked up this city";
    
    vector<int> ratings1 = {-1, -2, -5, -7, 11};
    vector<int> ratings2 = {-11, -22, -55, -76, 10};
    
    DocumentStatus status1 = DocumentStatus::ACTUAL;
    DocumentStatus status2 = DocumentStatus::BANNED;
    
    SearchServer search_server;
    search_server.AddDocument(1, query1, status1, ratings1);
    search_server.AddDocument(2, query2, status2, ratings2);
 
    vector<Document> document = search_server.FindTopDocuments("what happend with this city");
    ASSERT_EQUAL_HINT(document.size(), 1, "The number of matching documents is incorrect");
}
 
void TestMinusWords() {
    SearchServer search_server;
    
    int document_id = 1;
    vector<int> rating = {-1, -2, -5, -7, 11};
    DocumentStatus status = DocumentStatus::ACTUAL;
 
    string document_match = "tabby cat with big eyes";
    search_server.AddDocument(document_id, document_match, status, rating);
 
    document_id = 2;
    string document_minus = "small dog and tabby cat";
    search_server.AddDocument(document_id, document_minus, status, rating);
 
    string query = "-small -dog walks tabby cat";
    vector<Document> found_documents = search_server.FindTopDocuments(query);
    
    ASSERT_EQUAL_HINT(found_documents.size(), 1, "One document must be found");
    ASSERT_EQUAL_HINT(found_documents[0].id, 1, "Document id does not match");
}
 
void TestMatchWords(){
    SearchServer search_server;
    
    string document1 = "tabby cat with big eyes"s;
    vector<string> document_split = {"tabby"s, "cat"s, "with"s, "big"s, "eyes"s};
    sort(document_split.begin(), document_split.end());
    
    search_server.AddDocument(1, document1, DocumentStatus::ACTUAL, {13});
    tuple<vector<string>, DocumentStatus> matched_words1 = search_server.MatchDocument("tabby cat with big eyes"s, 1);
 
    ASSERT_EQUAL_HINT(document_split, get<vector<string>>(matched_words1), "Matching words must also be in the ethalon vector");
    
    string document2 = "small dog and tabby cat"s;
    search_server.AddDocument(2, document2, DocumentStatus::ACTUAL, {10});
    tuple<vector<string>, DocumentStatus> matched_words = search_server.MatchDocument("-small -dog and tabby cat"s, 2);
    const auto [string_, doc_] = matched_words;
    ASSERT_HINT(string_.empty(), "Matching words should not be found");
}
 
void TestSortByRelevance(){
    SearchServer search_server;
    int document_id = 1;
    vector<int> rating = {1, 2, 3};
    DocumentStatus status = DocumentStatus::ACTUAL;
    
    string document1 = "big elephant with pink super duper tail";
    search_server.AddDocument(document_id, document1, status, rating);
    document_id = 2;
    string document2 = "orange pig fucked in the pink mouth";
    search_server.AddDocument(document_id, document2, status, rating);
    document_id = 3;
    string document3 = "super pink dog with blue tail";
    search_server.AddDocument(document_id, document3, status, rating);
    document_id = 4;
    string document4 = "super pink carpet";
    search_server.AddDocument(document_id, document4, status, rating);
  
    string query = "pink super duper tail";
 
    vector<Document> found_documents = search_server.FindTopDocuments(query);
    ASSERT_EQUAL_HINT(found_documents[0].id, 1, "Document id does not match (relevance)");
    ASSERT_EQUAL_HINT(found_documents[1].id, 3, "Document id does not match (relevance)");
    ASSERT_EQUAL_HINT(found_documents[2].id, 4, "Document id does not match (relevance)");
    ASSERT_EQUAL_HINT(found_documents[3].id, 2, "Document id does not match (relevance)");
}
 
void TestRating() { 
    SearchServer search_server;
    DocumentStatus status = DocumentStatus::ACTUAL;
    
    int document_id = 1;
    vector<int> rating1 = {1, 1, 1, 1, 1};
    string document1 = "big elephant with pink super duper tail";
    search_server.AddDocument(document_id, document1, status, rating1);
    document_id = 2;
    vector<int> rating2 = {2, 2, 2, 2, 2};
    string document2 = "orange pig fucked in the pink mouth";
    search_server.AddDocument(document_id, document2, status, rating2);
    document_id = 3;
    vector<int> rating3 = {3, 3, 3, 3, 3};
    string document3 = "pink dog with blue collar and super tail";
    search_server.AddDocument(document_id, document3, status, rating3);
    document_id = 4;
    vector<int> rating4 = {4, 4, 4, 4, 4};
    string document4 = "super pink carpet";
    search_server.AddDocument(document_id, document4, status, rating4);
    document_id = 5;
    vector<int> rating5 = {5, 5, 5, 5, 5};
    string document5 = "green baby with pink big dick and sweet ass";
    search_server.AddDocument(document_id, document5, status, rating5);
  
    string query = "pink";
    
    vector<Document> documents_to_rating = search_server.FindTopDocuments(query);
    ASSERT_EQUAL_HINT(documents_to_rating[0].rating, 5, "Incorrect rating");
    ASSERT_EQUAL_HINT(documents_to_rating[1].rating, 4, "Incorrect rating");
    ASSERT_EQUAL_HINT(documents_to_rating[2].rating, 3, "Incorrect rating");
    ASSERT_EQUAL_HINT(documents_to_rating[3].rating, 2, "Incorrect rating");
    ASSERT_EQUAL_HINT(documents_to_rating[4].rating, 1, "Incorrect rating");
    
}
 
void TestFindDocumentByPredicate(){
    SearchServer search_server;
    search_server.AddDocument(1, "tabby cat with big eyes"s, DocumentStatus::REMOVED, {13});
    search_server.AddDocument(2, "small dog and tabby cat"s, DocumentStatus::ACTUAL, {20});
    vector<Document> found_document = search_server.FindTopDocuments("cat", []([[maybe_unused]] int document_id, DocumentStatus status, [[maybe_unused]] int rating) { return status == DocumentStatus::ACTUAL;});
    ASSERT_EQUAL_HINT(found_document[0].id, 2, "Incorrect document id (predicate)");
}
 
void TestFindDocumentByStatus(){
    SearchServer search_server;
    int document_id = 1;
    vector<int> rating = {1, 2, 3, 4, 5};
    DocumentStatus status1 = DocumentStatus::ACTUAL;
    string document1 = "big elephant with pink tail";
    search_server.AddDocument(document_id, document1, status1, rating);
    document_id = 2;
    string document2 = "pink pig fucked in the mouth";
    DocumentStatus status2 = DocumentStatus::BANNED;
    search_server.AddDocument(document_id, document2, status2, rating);
    document_id = 3;
    string document3 = "pink dog with blue collar";
    DocumentStatus status3 = DocumentStatus::IRRELEVANT;
    search_server.AddDocument(document_id, document3, status3, rating);
    document_id = 4;
    string document4 = "green baby with big pink dick and sweet ass";
    DocumentStatus status4 = DocumentStatus::REMOVED;
    search_server.AddDocument(document_id, document4, status4, rating);
    
    string query = "pink tail";
 
    vector<Document> documents_actual = search_server.FindTopDocuments(query, DocumentStatus::ACTUAL);
    ASSERT_EQUAL_HINT(documents_actual[0].id, 1, "Document id does not match (status)");
    
    vector<Document> documents_banned = search_server.FindTopDocuments(query, DocumentStatus::BANNED);
    ASSERT_EQUAL_HINT(documents_banned[0].id, 2, "Document id does not match (status)");
   
    vector<Document> documents_irrelevant = search_server.FindTopDocuments(query, DocumentStatus::IRRELEVANT);
    ASSERT_EQUAL_HINT(documents_irrelevant[0].id, 3, "Document id does not match (status)");
 
    vector<Document> documents_removed = search_server.FindTopDocuments(query, DocumentStatus::REMOVED);
    ASSERT_EQUAL_HINT(documents_removed[0].id, 4, "Document id does not match (status)");
}
 
void TestAccurateRelevance(){
    SearchServer search_server; 
    int document_id = 1; 
    vector<int> rating1 = { 1, 7, 13 }; 
    DocumentStatus status = DocumentStatus::ACTUAL; 
    string document1 = "big cute cat"s; 
    search_server.AddDocument(document_id, document1, status, rating1); 
 
    document_id = 2; 
    vector<int> rating2 = { 2, 4, 10 }; 
    string document2 = "small tabby bird"s; 
    search_server.AddDocument(document_id, document2, status, rating2); 
    vector<Document> document_ = search_server.FindTopDocuments("cat"s);
    ASSERT_EQUAL_HINT(document_[0].relevance, (log(2/1))*1/3, "Relevance does not match");   
}
 
// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMatchQuery);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatchWords);
    RUN_TEST(TestSortByRelevance);
    RUN_TEST(TestRating);
    RUN_TEST(TestFindDocumentByPredicate);
    RUN_TEST(TestFindDocumentByStatus);
    RUN_TEST(TestAccurateRelevance);
    // Не забудьте вызывать остальные тесты здесь
}
 
// --------- Окончание модульных тестов поисковой системы -----------
 
int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
