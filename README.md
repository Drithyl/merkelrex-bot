# merkelrex-bot

This repository was made to showcase the mid-term assignment for the Object-Oriented Programming module in the University of London Goldsmith's BSc in Computer Science, which was taught by Dr. Matthew Yee-King. Starting with a simple, console-based currency exchange simulator, we were tasked to modify it to be able to parse over a million CSV data entries of trades in an order book, and to design and run a simple trading strategy on them, creating bids and asks on the different products, process them or pull them back later when conditions change, while keeping track of a wallet and logging the activity. The original application required a significant amount of optimizations to achieve this within a reasonable time.

The full data file (/data/20200601.csv) is too large to be uploaded to GitHub and most other places (it crashes pastebin if trying to paste it there), and therefore has been cut to about one third of its original size here.

The main branch of the repository is the code as it was built with the professor throughout the course, and the starting point of our task. The submission branch is my final version of the program, and the submission which was made to the University. It scored 100/100 and is able to do go through the entire orderbook as requested within around 7 minutes, if considering every past order in the orderbook that was not completely fulfilled at each iteration, or only 7 seconds if only the orders with a given timestamp are considered at each step of the way.

The full report on the changes made to the original code, as it was submitted to the University, can be found in the "Midterm Report.pdf" file in the submission branch.
