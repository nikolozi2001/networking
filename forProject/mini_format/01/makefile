# პირველი მიზანი (target) სრულდება ავტომატურად, როცა უბრალოდ წერ 'make'
all: main

# პროგრამის კომპილაცია
main: main.c
	gcc main.c -o main

# კომპილაცია და გაშვება ერთად
run: main
	./main

# ფაილების გასუფთავება
clean:
	rm -f main
