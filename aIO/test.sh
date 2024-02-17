!# /bin/bash

#-----------------------------
# Sequential I/O, Piped files
#-----------------------------

# Small File
echo "AIO: sequential regular small file 1B"
./cat files/text1meg.txt > files/out.txt


echo "STDIO: sequential regular small file 1B"
./stdio-cat files/text1meg.txt > files/out.txt


echo "SLOW: sequential regular small file 1B"
./slow-cat files/text1meg.txt > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"

sleep 1

echo "AIO: sequential piped small file 1B"
cat files/text1meg.txt | ./cat | cat > files/out.txt


echo "STDIO: sequential piped small file 1B"
cat files/text1meg.txt | ./stdio-cat | cat > files/out.txt


echo "SLOW: sequential piped small file 1B"
cat files/text1meg.txt | ./slow-cat | cat > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"

sleep 1
# Mediumm File
echo "AIO: sequential regular medium file 1B"
./cat files/text5meg.txt > files/out.txt


echo "STDIO: sequential regular medium file 1B"
./stdio-cat files/text5meg.txt > files/out.txt


echo "SLOW: sequential regular medium file 1B"
./slow-cat files/text5meg.txt > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential piped medium file 1B"
cat files/text5meg.txt | ./cat | cat > files/out.txt


echo "STDIO: sequential piped medium file 1B"
cat files/text5meg.txt | ./stdio-cat | cat > files/out.txt


echo "SLOW: sequential piped medium file 1B"
cat files/text5meg.txt | ./slow-cat | cat > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
# Large File
echo "AIO: sequential regular large file 1B"
./cat files/text20meg.txt > files/out.txt


echo "STDIO: sequential regular large file 1B"
./stdio-cat files/text20meg.txt > files/out.txt


echo "SLOW: sequential regular large file 1B"
./slow-cat files/text20meg.txt > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential piped large file 1B"
cat files/text20meg.txt | ./cat | cat > files/out.txt


echo "STDIO: sequential piped large file 1B"
cat files/text20meg.txt | ./stdio-cat | cat > files/out.txt


echo "SLOW: sequential piped large file 1B"
cat files/text20meg.txt | ./slow-cat | cat > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
# BLOCK-CAT
echo "AIO: sequential regular medium file 1KB"
./blockcat -b 1024 files/text5meg.txt > files/out.txt


echo "STDIO: sequential regular medium file 1KB"
./stdio-blockcat -b 1024 files/text5meg.txt > files/out.txt


echo "SLOW: sequential regular medium file 1KB"
./slow-blockcat -b 1024 files/text5meg.txt > files/out.txt
sleep 1

echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential piped medium file 1KB"
cat files/text5meg.txt | ./blockcat -b 1024 | cat > files/out.txt


echo "STDIO: sequential piped medium file 1KB"
cat files/text5meg.txt | ./stdio-blockcat -b 1024 | cat > files/out.txt


echo "SLOW: sequential piped medium file 1KB"
cat files/text5meg.txt | ./slow-blockcat -b 1024 | cat > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential regular large file 1KB"
./blockcat -b 1024 files/text20meg.txt > files/out.txt


echo "STDIO: sequential regular large file 1KB"
./stdido-blockcat -b 1024 files/text20meg.txt > files/out.txt


echo "SLOW: sequential regular large file 1KB"
./slow-blockcat -b 1024 files/text20meg.txt > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential piped large file 1KB"
cat files/text20meg.txt | ./blockcat -b 1024 | cat > files/out.txt


echo "STDIO: sequential piped large file 1KB"
cat files/text20meg.txt | ./stdio-blockcat -b 1024 | cat > files/out.txt


echo "SLOW: sequential piped large file 1KB"
cat files/text20meg.txt | ./slow-blockcat -b 1024 | cat > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential regular large file 4KB"
./blockcat files/text20meg.txt > files/out.txt


echo "STDIO: sequential regular large file 4KB"
./stdio-blockcat files/text20meg.txt > files/out.txt


echo "SLOW: sequential regular large file 4KB"
./slow-blockcat files/text20meg.txt > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
echo "AIO: sequential piped large file 4KB"
cat files/text20meg.txt | ./blockcat | cat > files/out.txt


echo "STDIO: sequential piped large file 4KB"
cat files/text20meg.txt | ./stdio-blockcat | cat > files/out.txt


echo "SLOW: sequential piped large file 4KB"
cat files/text20meg.txt | ./slow-blockcat | cat > files/out.txt


echo "----------------------------------------"
echo "----------------------------------------"
sleep 1
# RANDOM BLOCK SIZES
echo "AIO: redirected large file, 1B-4KB block I/O, sequential"
./randomblockcat files/text20meg.txt > files/out.txt


echo "STDIO: redirected large file, 1B-4KB block I/O, sequential"
./stdio-randomblockcat files/text20meg.txt > files/out.txt


echo "SLOW: redirected large file, 1B-4KB block I/O, sequential"
./slow-randomblockcat files/text20meg.txt > files/out.txt


echo "----------------------END-TEST--------------------------"
