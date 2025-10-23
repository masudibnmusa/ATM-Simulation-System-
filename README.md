# 💳 ATM Simulation System (C Project)

## 📖 Overview
This is a simple **ATM Simulation System** written in **C language**.  
It allows users to log in using their account number and PIN, perform banking operations like deposits, withdrawals, and view transaction history.  
All account and transaction data are stored in text files (`accounts.txt` and `transactions.txt`).

---

## ⚙️ Features
- 🔐 User Login (Account Number & PIN)
- 💰 Deposit & Withdraw Money
- 📊 Balance Inquiry
- 🧾 Transaction History (Saved with timestamps)
- 💾 Persistent Data using File Handling

---

## 🧩 Concepts Used
- **Structures (`struct`)**
- **File Handling (Read/Write)**
- **Functions**
- **Basic Authentication**
- **Time & Date using `<time.h>`**

---

## 🗂️ Files
| File Name | Description |
|------------|-------------|
| `main.c` | Main program file |
| `accounts.txt` | Stores account details (Account No, Name, PIN, Balance) |
| `transactions.txt` | Stores transaction history with date and time |

---

## 🧮 Example Account Data Format (`accounts.txt`)
- 125742 Masud 1234 5000.00 
- 102013 Mahin 4321 3000.00
- 566778 Fahim 5678 2000.00
