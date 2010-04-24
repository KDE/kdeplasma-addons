import java.io.*;
import java.sql.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class KdeCommitsServlet extends HttpServlet
{
    public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException
    {
        String dbHost = "localhost";
        String dbName = "sandros_kde";
        String username = "sandros_kdeob";
        String password = "oBse_4s";

        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        out.println("<html>");
        out.println("<head>");
        out.println("<title>Hello World!</title>");
        out.println("</head>");
        out.println("<body>");
        out.println("<h1>Hello World!</h1>");

        try
        {
            Class.forName("com.mysql.jdbc.Driver");
        }
        catch(ClassNotFoundException msg)
        {
            out.println("Error loading driver:" + msg.getMessage());
        }

        try
        {
            String url ="jdbc:mysql://" + dbHost + ":3306/" + dbName;
            Connection Conn = DriverManager.getConnection(url, username, password);
            Statement Stmt = Conn.createStatement();
            String query = "select p.commit_subject, count(*) from projects p, commits c where INSTR(c.path, p.commit_subject) > 0 group by p.commit_subject order by count(*) desc";
            ResultSet res = Stmt.executeQuery(query);
            out.println("Query result :</br>");
            while(res.next())
            {
                out.println(res.getObject(1) + " - " + res.getObject(2) + "</br>");
            }
            out.println("</br>");
        }
        catch(SQLException e)
        {
            String err1Msg = e.getMessage();
        }

        out.println("</body>");
        out.println("</html>");
    }
}
