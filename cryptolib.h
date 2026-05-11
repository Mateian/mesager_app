#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <random> 
#ifdef CRYPTOLIB_EXPORTS
#define CRYPTO_API __declspec(dllexport)
#else
#define CRYPTO_API __declspec(dllimport)
#endif

/**
 * @class RSAEncryptor
 * @brief Clasă pentru criptare și decriptare RSA folosind numere prime definite de utilizator.
 */
class CRYPTO_API RSAEncryptor
{
private:
    std::string n;  
    std::string e; 
    std::string d;  

public:
    /**
     * @brief Constructorul clasei RSAEncryptor.
     * Inițializeaza cu 2 numere prime(obligatoriu).
     * Calculează n, e și d pentru criptare și decriptare.
     */
    RSAEncryptor(std::string prime1, std::string prime2);

    ~RSAEncryptor();

    /**
     * @brief Criptează un mesaj numeric sau hex.
     * Conversia din hex -> cpp_int se face intern.
     *
     * @param message mesajul numeric/hex ce trebuie criptat
     * @param publicKeyN cheia publica a destinatarului
     * @param publicKeyE exponentul public al destinatarului(optional este la fel)
     * @return mesajul criptat
     */
    std::string encrypt(std::string message, std::string publicKeyN, std::string publicKeyE = "65537");

    /**
     * @brief Decriptează un mesaj RSA.
     * Conversia din hex -> cpp_int și invers se face intern.
     *
     * @param cipher mesajul criptat în format hex
     * @return mesajul original
     */
    std::string decrypt(std::string cipher);

    /**
     * @brief Returnează cheia publică RSA.
     *
     * @return valoarea lui n
     */
    std::string getN();

    /**
     * @brief Returnează exponentul public RSA.
     *
     * @return valoarea lui e
     */
    std::string getE();
    std::string getD();
};

std::string toHex(const std::string& data);

/**
 * @class Blowfish
 * @brief Clasă pentru criptarea și decriptarea mesajelor folosind algoritmul Blowfish simplificat (XOR demo).
 */
class CRYPTO_API Blowfish
{
private:
    std::string key; 
    std::vector<std::vector<uint32_t>> S;
    std::vector<uint32_t> P;

    uint32_t F(uint32_t x);
    void encryptBlock(uint32_t& L, uint32_t& R);
    void decryptBlock(uint32_t& L, uint32_t& R);

    void expandKey();

public:
    uint32_t encryptBlock(uint32_t block);
    uint32_t decryptBlock(uint32_t block);

    /**
     * @brief Constructorul clasei Blowfish.
     * Setează cheia folosită pentru criptare/decriptare.
     * Limitată superior la 56 de caractere și inferior la 4(autocompletate random)
     * @param key cheia Blowfish
     */
    Blowfish(std::string key);

    ~Blowfish();

    /**
     * @brief Returnează cheia Blowfish curentă.
     *
     * @return cheia de criptare
     */
    std::string getKey();

    /**
     * @brief Criptează un mesaj.
     * @param message mesajul ce trebuie criptat.
     * @return mesajul criptat
     */
    std::string encryptMessage(std::string message);

    /**
     * @brief Decriptează un mesaj.
     * @param cipher mesajul criptat
     * @return mesajul original
     */
    std::string decryptMessage(std::string cipher);
};