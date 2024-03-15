module linkedList
    public class Node inherits Object
        public field-Node next := nil;
        public field-Node prev := nil;
        public field-integer data;
    end Node;

    public class List inherits Object
        private field-Node head := nil;
        private field-Node tail := nil;

        public method add(List this)(in integer data)
            variable-Node newNode;
            let newNode.data := data;
            if this.head == nil then
                let this.head := newNode;
                let this.tail := newNode;
            else
                let this.tail.next := newNode;
                let newNode.prev := this.tail;
                let this.tail := newNode;
            end if;
        end add;

        public method write(List this)()
            variable-Node current := this.head;
            while current != nil repeat
                output f"{current.data} -> ";
                let current := current.next;
            end while;
            output "nil";
        end write;

        public method destroy(List this)()
            variable-Node current := this.head;
            while current != nil repeat
                variable-Node temp := current.next;
                delete current;
                let current := temp;
            end while;
            let this.head := nil;
            let this.tail := nil;
        end destroy;

        public method insert(List this)(in integer index, in integer data)
            variable-Node newNode;
            let newNode.data := data;
            if index <= 0 then
                // Insert at the beginning
                let newNode.next := this.head;
                if this.head != nil then
                    let this.head.prev := newNode;
                end if;
                let this.head := newNode;
                if this.tail == nil then
                    let this.tail := newNode;
                end if;
            else
                variable-Node current := this.head;
                variable-integer currentIndex := 0;
                while current != nil repeat
                    let currentIndex := currentIndex + 1;
                    if currentIndex == index then
                        // Insert in the middle or at the end
                        let newNode.next := current.next;
                        let newNode.prev := current;
                        if current.next != nil then
                            let current.next.prev := newNode;
                        end if;
                        let current.next := newNode;
                        if newNode.next == nil then
                            let this.tail := newNode;
                        end if;
                        return;
                    end if;
                    let current := current.next;
                end while;
            end if;
        end insert;

        public method erase(List this)(in integer index)
            if index < 0 then
                return; // Invalid index, do nothing
            end if;

            variable-Node current := this.head;
            variable-integer currentIndex := 0;

            // Special case for erasing the head node
            if index == 0 then
                let this.head := current.next;
                if this.head != nil then
                    let this.head.prev := nil;
                end if;
                delete current;
                return;
            end if;


            // Traverse the list to find the node to be erased
            while current != nil repeat
                if currentIndex == index then
                    // Erase the current node
                    let current.prev.next := current.next;
                    if current.next != nil then
                        let current.next.prev := current.prev;
                    else
                        // Update the tail if the erased node was the last node
                        let this.tail := current.prev;
                    end if;
                    delete current;
                    return;
                end if;
                let current := current.next;
                let currentIndex := currentIndex + 1;
            end while;
        end erase;

    end List;

start
    variable-List list;
    call list.add(1);
    call list.add(2);
    call list.write(); // Output: 1 -> 2 -> nil
    output "\n";

    call list.insert(1, 3);
    call list.write(); // Output: 1 -> 3 -> 2 -> nil
    output "\n";

    call list.insert(0, 0);
    call list.write(); // Output: 0 -> 1 -> 3 -> 2 -> nil
    output "\n";

    call list.insert(4, 4);
    call list.write(); // Output: 0 -> 1 -> 3 -> 2 -> 4 -> nil
    output "\n";

    call list.erase(1);
    call list.write(); // Output: 0 -> 3 -> 2 -> 4 -> nil
    output "\n";

    call list.erase(0);
    call list.write(); // Output: 3 -> 2 -> 4 -> nil
    output "\n";

    call list.erase(2);
    call list.write(); // Output: 3 -> 2 -> nil
    output "\n";

    call list.destroy();
    delete list;
end linkedList.
