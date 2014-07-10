DO $$
declare
	accountid int;
	accountname varchar(50);
begin

FOR i IN 100014..1000013 LOOP
accountid := i;
select 'test' || i -2 into accountname;
insert into account values (accountid, accountname, '613b5247e3398350918cb622a3ec19e9');
END LOOP;

end;
$$language plpgsql;
