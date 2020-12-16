1. Add new key
If you want to add a key for new customer, launch keys.bat. Then you must input customer name.
When keys.bat is finished, you can see sub directory the name is same as customer name.
You can find keys.txt in that sub directory. When you open keys.txt, you can see customer name, provider id and keys.
You have to add them to BasicEncryptionProviderKeySet class in NexEditorSDK and nexassetpackagecriper.

2. Encrypt
java -jar nexassetpackagecryper.jar enc "customer name" "source file path" "destination file path"
ex: java -jar nexassetpackagecryper.jar enc LG 100.zip 100_LG.zip

3. Decrypt
java -jar nexassetpackagecryper.jar dec "customer name" "source file path" "destination file path"
ex: java -jar nexassetpackagecryper.jar dec LG 100_LG.zip 100_clean.zip

4. Reference document
https://docs.google.com/document/d/1Qs6WnBo96K4EnelgIIWN_4NZ0gzbRg2onjuJrh9nckg
https://docs.google.com/document/d/1FR-b5TLPEB6uAqYa39ILZxqIXPrFmCLorYvy2gXaOOc

Remarks
2017.02.27 key list
HUAWEI
LG
NEX
OPPO
SEC
TINNO
VIVO
XIAOMI
ZTE